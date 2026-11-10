#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../settings/elements.h"
#include "../settings/variables.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../image_loaders/image_dx11.h"
#include "../data/icons.h"

// Static map to store animation progress for each bone
static std::unordered_map<std::string, float> bone_anim_progress;

bool c_widgets::bone_selector_preview(const char* str_id, const char* prefix, float preview_alpha)
{
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems) return false;

    const ImVec2 size = scale(230, 420);
    bool value_changed = false;
    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(str_id);
    const ImVec2 pos = window->DC.CursorPos;

    const ImRect bb(pos, pos + size);
    gui->item_size(bb);

    const float char_w = scale(180);
    const float char_h = scale(306);
    const float char_x = (size.x - char_w) * 0.5f;
    const float char_y = scale(35);
    
    const ImVec2 char_pos = ImVec2(char_x, char_y);
    const ImVec2 char_size = ImVec2(char_w, char_h);
    const float dt = ImGui::GetIO().DeltaTime;

    // Draw character image (background)
    if (image->get("character"))
    {
        const ImVec2 img_min = pos + char_pos;
        const ImVec2 img_max = img_min + char_size;
        drawlist->AddImage(image->get("character"), img_min, img_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, (int)(230 * preview_alpha)));
    }
    
    // Draw skeleton image
    if (image->get("skeleton"))
    {
        const float skel_scale = char_h / 243.f * 0.75f;
        const float skel_w = 144.f * skel_scale;
        const float skel_h = 243.f * skel_scale;
        const float skel_x = char_x + (char_w - skel_w) * 0.5f;
        const float skel_y = char_y + (char_h - skel_h) * 0.5f + scale(12);
        const ImVec2 skel_min = pos + ImVec2(skel_x, skel_y);
        const ImVec2 skel_max = skel_min + ImVec2(skel_w, skel_h);
        
        const ImU32 skel_tint = IM_COL32(255, 255, 255, (int)(255 * preview_alpha));
        drawlist->AddImage(image->get("skeleton"), skel_min, skel_max, ImVec2(0, 0), ImVec2(1, 1), skel_tint);
        
        // Bone positions (fine-tuned based on user feedback v8)
        const float head_y = -0.08f;     // Kafa - aynı
        const float neck_y = 0.02f;      // Boyun - aynı
        const float shoulder_y = 0.08f;  // Omuzlar - biraz daha yukarı
        const float chest_y = 0.24f;     // Göğüs - aynı
        const float stomach_y = 0.38f;   // Karın - aynı
        const float hand_y = 0.41f;      // Eller - yukarı alındı
        const float hip_y = 0.52f;       // Kalça - aynı
        const float foot_y = 0.98f;      // Ayaklar - aynı
        
        const float shoulder_x = 0.19f;  // Omuz genişliği - biraz daha içeri
        const float hand_x = 0.61f;      // El genişliği - daha DIŞARI (L sola, R sağa)
        const float hip_x = 0.12f;       // Kalça genişliği - aynı
        const float left_foot_x = 0.17f; // Sol ayak - biraz daha sola
        const float right_foot_x = 0.14f;// Sağ ayak - aynı konumda
        
        ImVec2 head_pos = ImVec2(skel_min.x + skel_w * 0.5f, skel_min.y + skel_h * head_y);
        ImVec2 neck_pos = ImVec2(skel_min.x + skel_w * 0.5f, skel_min.y + skel_h * neck_y);
        ImVec2 chest_pos = ImVec2(skel_min.x + skel_w * 0.5f, skel_min.y + skel_h * chest_y);
        ImVec2 stomach_pos = ImVec2(skel_min.x + skel_w * 0.5f, skel_min.y + skel_h * stomach_y);
        ImVec2 left_shoulder_pos = ImVec2(skel_min.x + skel_w * (0.5f - shoulder_x), skel_min.y + skel_h * shoulder_y);
        ImVec2 right_shoulder_pos = ImVec2(skel_min.x + skel_w * (0.5f + shoulder_x), skel_min.y + skel_h * shoulder_y);
        ImVec2 left_hand_pos = ImVec2(skel_min.x + skel_w * (0.5f - hand_x), skel_min.y + skel_h * hand_y);
        ImVec2 right_hand_pos = ImVec2(skel_min.x + skel_w * (0.5f + hand_x), skel_min.y + skel_h * hand_y);
        ImVec2 left_hip_pos = ImVec2(skel_min.x + skel_w * (0.5f - hip_x), skel_min.y + skel_h * hip_y);
        ImVec2 right_hip_pos = ImVec2(skel_min.x + skel_w * (0.5f + hip_x), skel_min.y + skel_h * hip_y);
        ImVec2 left_foot_pos = ImVec2(skel_min.x + skel_w * (0.5f - left_foot_x), skel_min.y + skel_h * foot_y);
        ImVec2 right_foot_pos = ImVec2(skel_min.x + skel_w * (0.5f + right_foot_x), skel_min.y + skel_h * foot_y);

        // Bone button with open/close animation
        auto draw_bone_button = [&](const char* bone_name, ImVec2 bone_pos, const char* tooltip, bool is_left_side = false) -> bool {
            char config_key[64];
            snprintf(config_key, sizeof(config_key), "%s_bone_%s", prefix, bone_name);
            
            bool& enabled = var->cfg.get<bool>(config_key);
            
            // Get/create animation progress for this bone
            std::string anim_key = std::string(prefix) + "_" + bone_name;
            float& anim = bone_anim_progress[anim_key];
            
            // Animate towards target (1.0 = enabled, 0.0 = disabled)
            float target = enabled ? 1.0f : 0.0f;
            float anim_speed = 8.0f;
            if (anim < target) {
                anim = ImMin(anim + dt * anim_speed, target);
            } else if (anim > target) {
                anim = ImMax(anim - dt * anim_speed, target);
            }
            
            // Sabit boyutlu buton - açık/kapalı fark etmez aynı boyut
            const float outer_radius = scale(6.0f);
            const float inner_radius = scale(3.0f);
            const float hover_add = scale(1.5f);
            
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;
            float dist = sqrtf((mouse_pos.x - bone_pos.x) * (mouse_pos.x - bone_pos.x) + 
                               (mouse_pos.y - bone_pos.y) * (mouse_pos.y - bone_pos.y));
            bool hovered = dist <= outer_radius + hover_add;
            bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            
            if (clicked) {
                enabled = !enabled;
                value_changed = true;
            }
            
            float current_outer = outer_radius;
            if (hovered) current_outer += hover_add;
            
            // Renkler - dış çerçeve her zaman gri
            ImU32 outer_color = IM_COL32(70, 70, 70, (int)(220 * preview_alpha));
            ImU32 fill_color = IM_COL32(35, 35, 35, (int)(200 * preview_alpha));
            ImU32 inner_color = IM_COL32(255, 255, 255, (int)(255 * anim * preview_alpha));
            
            // Hover efekti
            if (hovered) {
                outer_color = IM_COL32(100, 100, 100, (int)(240 * preview_alpha));
                fill_color = IM_COL32(50, 50, 50, (int)(220 * preview_alpha));
            }
            
            // Dış daire (gri dolgu + gri çerçeve)
            drawlist->AddCircleFilled(bone_pos, current_outer, fill_color);
            drawlist->AddCircle(bone_pos, current_outer, outer_color, 0, scale(1.5f));
            
            // İç beyaz daire (sadece açıkken görünür, animasyonlu)
            if (anim > 0.01f) {
                float animated_inner = inner_radius * anim;
                drawlist->AddCircleFilled(bone_pos, animated_inner, inner_color);
            }
            
            // Tooltip - position based on which side the bone is on
            if (hovered) {
                ImVec2 text_size = ImGui::CalcTextSize(tooltip);
                ImVec2 tooltip_size = text_size + ImVec2(scale(10), scale(6));
                ImVec2 tooltip_pos;
                
                if (is_left_side) {
                    // Left side bones: show tooltip on the right
                    tooltip_pos = ImVec2(bone_pos.x + scale(14), bone_pos.y - tooltip_size.y * 0.5f);
                } else {
                    // Right side bones: show tooltip on the left
                    tooltip_pos = ImVec2(bone_pos.x - scale(14) - tooltip_size.x, bone_pos.y - tooltip_size.y * 0.5f);
                }
                
                drawlist->AddRectFilled(tooltip_pos, ImVec2(tooltip_pos.x + tooltip_size.x, tooltip_pos.y + tooltip_size.y), 
                    IM_COL32(15, 15, 15, (int)(240 * preview_alpha)), scale(4));
                drawlist->AddRect(tooltip_pos, ImVec2(tooltip_pos.x + tooltip_size.x, tooltip_pos.y + tooltip_size.y), 
                    IM_COL32(80, 80, 80, (int)(200 * preview_alpha)), scale(4));
                drawlist->AddText(ImVec2(tooltip_pos.x + scale(5), tooltip_pos.y + scale(3)), 
                    IM_COL32(255, 255, 255, (int)(255 * preview_alpha)), tooltip);
            }
            return enabled;
        };

        // Draw bone buttons (no knee bones)
        // Center bones - tooltip on right side
        draw_bone_button("head", head_pos, "Head", true);
        draw_bone_button("neck", neck_pos, "Neck", true);
        draw_bone_button("chest", chest_pos, "Chest", true);
        draw_bone_button("stomach", stomach_pos, "Stomach", true);
        // Left side bones - tooltip on right side
        draw_bone_button("left_shoulder", left_shoulder_pos, "L.Shoulder", true);
        draw_bone_button("left_hand", left_hand_pos, "L.Hand", true);
        draw_bone_button("left_hip", left_hip_pos, "L.Hip", true);
        draw_bone_button("left_foot", left_foot_pos, "L.Foot", true);
        // Right side bones - tooltip on left side
        draw_bone_button("right_shoulder", right_shoulder_pos, "R.Shoulder", false);
        draw_bone_button("right_hand", right_hand_pos, "R.Hand", false);
        draw_bone_button("right_hip", right_hip_pos, "R.Hip", false);
        draw_bone_button("right_foot", right_foot_pos, "R.Foot", false);
    }

    // Count active bones (no knee)
    int active_count = 0;
    const char* bone_names[] = {"head", "neck", "chest", "stomach", "left_shoulder", "right_shoulder", 
                                "left_hand", "right_hand", "left_hip", "right_hip", "left_foot", "right_foot"};
    for (const char* bone : bone_names) {
        char key[64];
        snprintf(key, sizeof(key), "%s_bone_%s", prefix, bone);
        if (var->cfg.get<bool>(key)) active_count++;
    }

    // Mode text
    char mode_text[32];
    if (active_count == 0) snprintf(mode_text, sizeof(mode_text), "No bones selected");
    else if (active_count == 1) snprintf(mode_text, sizeof(mode_text), "Single bone");
    else snprintf(mode_text, sizeof(mode_text), "Closest (%d bones)", active_count);
    
    ImVec2 mode_size = ImGui::CalcTextSize(mode_text);
    ImVec2 mode_pos = ImVec2(pos.x + (size.x - mode_size.x) * 0.5f, pos.y + size.y - scale(30));
    drawlist->AddText(mode_pos, IM_COL32(220, 220, 220, (int)(255 * preview_alpha)), mode_text);

    const char* info_text = "Click bones to toggle";
    ImVec2 info_size = ImGui::CalcTextSize(info_text);
    ImVec2 info_pos = ImVec2(pos.x + (size.x - info_size.x) * 0.5f, pos.y + size.y - scale(15));
    drawlist->AddText(info_pos, IM_COL32(140, 140, 140, (int)(180 * preview_alpha)), info_text);

    return value_changed;
}
