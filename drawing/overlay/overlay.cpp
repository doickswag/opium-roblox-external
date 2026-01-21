#include "overlay.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_dx11.h"

// Removed: Text editor include (was only used for the Lua tab)
#include <dwmapi.h>
#include <numeric>
#include <shellapi.h>

#include <filesystem>
#include <thread>
#include <chrono>
#include <bitset>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include "../../util/classes/classes.h"

using namespace roblox;
#include <cmath>
#include <unordered_set>
#include <vector>
#include <cctype>
#include <string>

#ifdef min
#undef min
#endif
#include <stack>
#include "../../util/notification/notification.h"
#ifdef max
#undef max
#endif
#include <Psapi.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_dx11.h"
#include "../imgui/misc/freetype/imgui_freetype.h"
#include "../imgui/addons/imgui_addons.h"
#include <dwmapi.h>
#include <d3dx11.h>
#include "../../util/globals.h"
#include "keybind/keybind.h"
#include "../../features/visuals/visuals.h"
#include "../../util/config/configsystem.h"
#include "../../features/combat/modules/dahood/autostuff/auto.h"
#include "../../features/wallcheck/wallcheck.h"

// Global UI theme: dark background with red accents inspired by SKETCH-style menus
void SetBlackBlueTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Core palette (sRGB-ish)
    ImVec4 col_bg_dark = ImVec4(0.06f, 0.06f, 0.06f, 1.00f); // main window background
    ImVec4 col_bg_panel = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // inner panels / children
    ImVec4 col_bg_popup = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    ImVec4 col_text = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    ImVec4 col_text_disabled = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);

    // Accent purple used across the UI
    ImVec4 col_blue = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // primary accent (purple)
    ImVec4 col_blue_hover = ImVec4(0.80f, 0.50f, 1.00f, 1.00f);
    ImVec4 col_blue_active = ImVec4(0.60f, 0.30f, 1.00f, 1.00f);

    // --- Backgrounds ---
    colors[ImGuiCol_WindowBg] = col_bg_dark;
    colors[ImGuiCol_ChildBg] = col_bg_panel;
    colors[ImGuiCol_PopupBg] = col_bg_popup;

    // --- Text ---
    colors[ImGuiCol_Text] = col_text;
    colors[ImGuiCol_TextDisabled] = col_text_disabled;

    // --- Borders / outlines ---
    colors[ImGuiCol_Border] = ImVec4(0.239, 0.075, 0.380, 1.0); // purple-tinted border
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // --- Frames (inputs / combo / slider track) ---
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    // --- Buttons / toggles ---
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = col_blue_hover;
    colors[ImGuiCol_ButtonActive] = col_blue_active;

    // --- Tabs ---
    colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TabHovered] = col_blue_hover;
    colors[ImGuiCol_TabActive] = col_blue;

    // --- Sliders / grabs ---
    colors[ImGuiCol_SliderGrab] = col_blue;
    colors[ImGuiCol_SliderGrabActive] = col_blue_active;

    // Checkmarks / selected states
    colors[ImGuiCol_CheckMark] = col_blue;

    // Rounded borders
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
}




// Waypoint system removed
// Minimal stubs to satisfy references in disabled waypoint UI/render functions
struct WaypointStub {
    std::string name;
    math::Vector3 position;
    bool visible;
    WaypointStub(const std::string& n, const math::Vector3& p) : name(n), position(p), visible(true) {}
};
static std::vector<WaypointStub> waypoints;
static bool show_waypoint_panel = false;
static char new_waypoint_name[128] = "";

// World tab features (Conditions and Customization)
static bool world_god_mode = false;
static bool world_sem_god_mode = false;
static bool world_invisible = false;
static bool world_super_jump = false;
static bool world_infinite_stamina = false;
static bool world_no_ragdoll = false;
static bool world_noclip = false;
static bool world_invisible_noclip = false;
static bool world_run_speed = false;
static bool world_swim_speed = false;
static bool world_never_wanted = false;
static bool world_no_collision = false;

static float world_noclip_speed = 1.00f;
static float world_run_speed_multiplier = 1.00f;
static float world_swim_speed_multiplier = 1.00f;
static int world_noclip_mode = 0; // 0 = Direction
static float world_health_amount = 100.0f;
static float world_armor_amount = 100.0f;

// Roblox Instance Explorer functionality (like Dex)
static instance selected_instance;
static std::unordered_set<uint64_t> expanded_instances;
static std::unordered_map<uint64_t, std::vector<instance>> instance_cache;
static std::unordered_map<uint64_t, std::string> instance_name_cache;
static std::unordered_map<uint64_t, std::string> instance_class_cache;
static std::unordered_map<uint64_t, std::string> instance_path_cache;
static std::unordered_map<uint64_t, bool> instance_children_loaded; // Track if children are loaded
static char search_filter[256] = "";
static std::vector<instance> search_results;
static bool show_search_results = false;
static bool cache_initialized = false;
static auto last_cache_refresh = std::chrono::steady_clock::now();
static auto last_tree_render = std::chrono::steady_clock::now();

// Filter options
static bool show_only_parts = false;
static bool show_only_scripts = false;
static bool search_by_path = false; // Add search by path option

// Double-click detection
static std::string last_clicked_item = "";
static auto last_click_time = std::chrono::steady_clock::now();
static const auto double_click_duration = std::chrono::milliseconds(300);



// Performance optimization constants
static const int CACHE_REFRESH_INTERVAL_SECONDS = 10; // Increased from 2 to 10 seconds
static const int MAX_CACHE_SIZE = 10000; // Limit cache size to prevent memory issues

// Forward declarations
static void safe_teleport_to(const math::Vector3& rawTarget);
static std::string getInstanceName(const instance& instance);
static std::string getInstanceClassName(const instance& instance);
static std::string getInstanceDisplayName(const instance& instance);
static std::string getInstancePath(const instance& instance);
static std::string getInstanceFullPath(const instance& instance);
static void searchInstances(const instance& instance, const std::string& query);
static void renderInstanceTree(const instance& instance, int depth);
static void render_explorer();

bool isAutoFunctionActivez() {
    return globals::bools::bring || globals::bools::kill || globals::bools::autokill;
}

// Helper functions for instance explorer

static void cacheInstance(const instance& instance, const std::string& path) {
    if (instance.address == 0) return;

    try {
        std::string name = instance.get_name();
        std::string class_name = instance.get_class_name();

        // Apply filters
        if (show_only_parts && class_name != "Part" && class_name != "BasePart" && class_name != "Model") {
            return; // Skip non-parts
        }

        if (show_only_scripts && class_name != "Script" && class_name != "LocalScript" && class_name != "ModuleScript") {
            return; // Skip non-scripts
        }

        instance_name_cache[instance.address] = name;
        instance_class_cache[instance.address] = class_name;
        instance_path_cache[instance.address] = path;

        // Only cache immediate children, not recursively (lazy loading)
        std::vector<roblox::instance> children = instance.get_children();
        instance_cache[instance.address] = children;
        instance_children_loaded[instance.address] = true;

        // Limit cache size to prevent memory issues
        if (instance_cache.size() > MAX_CACHE_SIZE) {
            // Remove oldest entries
            auto it = instance_cache.begin();
            instance_cache.erase(it);
            instance_name_cache.erase(it->first);
            instance_class_cache.erase(it->first);
            instance_path_cache.erase(it->first);
            instance_children_loaded.erase(it->first);
        }
    }
    catch (...) {
        // Ignore errors for individual instances
    }
}

static void searchInstances(const instance& instance, const std::string& query) {
    if (instance.address == 0) return;

    try {
        std::string name = instance.get_name();
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });

        // Search by name (default behavior)
        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), [](unsigned char c) { return std::tolower(c); });

        if (name.find(lower_query) != std::string::npos) {
            search_results.push_back(instance);
        }

        // If path search is enabled, also search by path
        if (search_by_path) {
            std::string path = getInstancePath(instance);
            std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) { return std::tolower(c); });

            if (path.find(lower_query) != std::string::npos) {
                // Only add if not already in results
                bool already_found = false;
                for (const auto& existing : search_results) {
                    if (existing.address == instance.address) {
                        already_found = true;
                        break;
                    }
                }
                if (!already_found) {
                    search_results.push_back(instance);
                }
            }
        }

        // Search children
        auto children = instance.get_children();
        for (const auto& child : children) {
            searchInstances(child, query);
        }
    }
    catch (...) {
        // Ignore search errors
    }
}

static std::string getInstanceName(const instance& instance) {
    if (instance.address == 0) return "Invalid";

    auto it = instance_name_cache.find(instance.address);
    if (it != instance_name_cache.end()) {
        return it->second;
    }

    try {
        return instance.get_name();
    }
    catch (...) {
        return "Unknown";
    }
}

static std::string getInstanceClassName(const instance& instance) {
    if (instance.address == 0) return "Invalid";

    auto it = instance_class_cache.find(instance.address);
    if (it != instance_class_cache.end()) {
        return it->second;
    }

    try {
        return instance.get_class_name();
    }
    catch (...) {
        return "Unknown";
    }
}

static std::string getInstanceDisplayName(const instance& instance) {
    if (instance.address == 0) return "Invalid";

    std::string name = getInstanceName(instance);
    std::string class_name = getInstanceClassName(instance);

    return name + " [" + class_name + "]";
}

static std::string getInstancePath(const instance& instance) {
    if (instance.address == 0) return "Invalid";

    auto it = instance_path_cache.find(instance.address);
    if (it != instance_path_cache.end()) {
        return it->second;
    }

    // Build path manually if not cached - format matches Dex exactly
    try {
        std::string path = instance.get_name();
        roblox::instance parent = instance.read_parent();
        while (parent.address != 0) {
            std::string parent_name = parent.get_name();
            if (!parent_name.empty()) {
                path = parent_name + "." + path;
            }
            parent = parent.read_parent();
        }

        // Cache the result
        instance_path_cache[instance.address] = path;
        return path;
    }
    catch (...) {
        return "Unknown";
    }
}

// Enhanced path function that includes class names like Dex
static std::string getInstanceFullPath(const instance& instance) {
    if (instance.address == 0) return "Invalid";

    try {
        std::string path = instance.get_name() + " (" + instance.get_class_name() + ")";
        roblox::instance parent = instance.read_parent();
        while (parent.address != 0) {
            std::string parent_name = parent.get_name();
            std::string parent_class = parent.get_class_name();
            if (!parent_name.empty()) {
                path = parent_name + " (" + parent_class + ")." + path;
            }
            parent = parent.read_parent();
        }
        return path;
    }
    catch (...) {
        return "Unknown";
    }
}

static void renderInstanceTree(const instance& instance, int depth) {
    if (instance.address == 0) return;

    try {
        std::string name = getInstanceName(instance);
        std::string class_name = getInstanceClassName(instance);
        std::string display_name = name + " [" + class_name + "]";

        bool is_selected = (selected_instance.address == instance.address);
        bool has_children = false;
        bool is_expanded = expanded_instances.count(instance.address);

        // Check if we need to load children (lazy loading)
        if (instance_children_loaded.count(instance.address) == 0) {
            // Load children only when needed
            try {
                std::vector<roblox::instance> children = instance.get_children();
                instance_cache[instance.address] = children;
                instance_children_loaded[instance.address] = true;
                has_children = !children.empty();
            }
            catch (...) {
                has_children = false;
            }
        }
        else {
            has_children = !instance_cache[instance.address].empty();
        }

        ImGui::PushID(instance.address);

        // Indent based on depth
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + depth * 20.0f);

        // Expand/collapse arrow for instances with children
        if (has_children) {
            if (ImGui::ArrowButton(("##expand" + std::to_string(instance.address)).c_str(),
                is_expanded ? ImGuiDir_Down : ImGuiDir_Right)) {
                if (is_expanded) {
                    expanded_instances.erase(instance.address);
                }
                else {
                    expanded_instances.insert(instance.address);
                }
            }
            ImGui::SameLine();
        }
        else {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f); // Align with expandable items
        }



        // Instance name
        if (ImGui::Selectable(display_name.c_str(), is_selected)) {
            selected_instance = instance;
        }

        // Quick copy path button (small and unobtrusive)
        ImGui::SameLine();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
        if (ImGui::Button(("##copy" + std::to_string(instance.address)).c_str(), ImVec2(16, 16))) {
            std::string path = getInstancePath(instance);
            ImGui::SetClipboardText(path.c_str());
        }

        ImGui::PopStyleVar();

        // Handle double-click
        if (ImGui::IsItemClicked()) {
            auto now = std::chrono::steady_clock::now();
            if (last_clicked_item == std::to_string(instance.address) &&
                (now - last_click_time) < double_click_duration) {
                // Double-click - expand/collapse
                if (has_children) {
                    if (is_expanded) {
                        expanded_instances.erase(instance.address);
                    }
                    else {
                        expanded_instances.insert(instance.address);
                    }
                }
                last_clicked_item = "";
            }
            else {
                selected_instance = instance;
                last_clicked_item = std::to_string(instance.address);
                last_click_time = now;
            }
        }

        // Right-click context menu
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            selected_instance = instance;
            ImGui::OpenPopup("InstanceContextMenu");
        }

        ImGui::PopID();

        // Render children if expanded (simplified approach)
        if (is_expanded && has_children) {
            // Only limit rendering for very large instance trees
            if (instance_cache[instance.address].size() > 500) {
                // For very large trees, show first 100 instances and a summary
                int max_to_show = 100;
                int total_children = instance_cache[instance.address].size();

                for (int i = 0; i < std::min(max_to_show, total_children); i++) {
                    renderInstanceTree(instance_cache[instance.address][i], depth + 1);
                }

                if (total_children > max_to_show) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "... (%d more instances)", total_children - max_to_show);
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Large instance tree - consider using search filters");
                }
            }
            else {
                // For normal sized trees, render all children
                for (const auto& child : instance_cache[instance.address]) {
                    renderInstanceTree(child, depth + 1);
                }
            }
        }

    }
    catch (...) {
        // Ignore rendering errors for individual instances
    }
}

static void render_explorer() {
    ImGui::BeginChild("ExplorerContent", ImVec2(0, 0), true);

    // Header
    ImGui::Text("Roblox Instance Explorer");

    ImGui::SameLine();

    // Search and filter controls
    ImGui::InputTextWithHint("##SearchFilter", "Search instances...", search_filter, sizeof(search_filter));

    ImGui::SameLine();
    if (ImGui::Checkbox("Search by Path", &search_by_path)) {
        // Refresh search when changing search mode
        if (strlen(search_filter) > 0) {
            search_results.clear();
            show_search_results = false;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        cache_initialized = false;
        search_results.clear();
        show_search_results = false;
        selected_instance = instance();
        expanded_instances.clear();
        instance_children_loaded.clear(); // Clear lazy loading flags
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear Cache")) {
        instance_cache.clear();
        instance_name_cache.clear();
        instance_class_cache.clear();
        instance_path_cache.clear();
        instance_children_loaded.clear();
        cache_initialized = false;
        search_results.clear();
        show_search_results = false;
        selected_instance = instance();
        expanded_instances.clear();
    }

    ImGui::SameLine();
    if (ImGui::Button("Copy All Paths")) {
        std::string all_paths;
        for (const auto& [addr, children] : instance_cache) {
            try {
                roblox::instance inst(addr);
                if (inst.address != 0) {
                    std::string path = getInstancePath(inst);
                    all_paths += path + "\n";
                }
            }
            catch (...) {
                // Skip invalid instances
            }
        }
        if (!all_paths.empty()) {
            ImGui::SetClipboardText(all_paths.c_str());
        }
    }

    ImGui::SameLine();
    if (ImGui::Checkbox("Show Only Parts", &show_only_parts)) {
        // Refresh cache when filter changes
        cache_initialized = false;
    }

    ImGui::SameLine();
    if (ImGui::Checkbox("Show Only Scripts", &show_only_scripts)) {
        // Refresh cache when filter changes
        cache_initialized = false;
    }



    ImGui::Separator();

    // Instance tree view and properties
    ImGui::Columns(2, nullptr, false);

    // Left column - Instance tree
    ImGui::BeginChild("InstanceTree", ImVec2(0, 0), true);

    try {
        // Get the DataModel (game)
        auto& datamodel = globals::instances::datamodel;
        if (datamodel.address == 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DataModel not found. Make sure you're in a Roblox game.");
            ImGui::EndChild();
            ImGui::EndChild();
            return;
        }

        instance root_instance(datamodel.address);

        // Cache refresh logic - reduced frequency for better performance
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_cache_refresh).count() >= CACHE_REFRESH_INTERVAL_SECONDS) {
            // Only clear caches if they're getting too large
            if (instance_cache.size() > MAX_CACHE_SIZE * 0.8) {
                instance_cache.clear();
                instance_name_cache.clear();
                instance_class_cache.clear();
                instance_path_cache.clear();
                instance_children_loaded.clear();
                cache_initialized = false;
            }
            last_cache_refresh = now;
        }

        // Initialize cache if needed
        if (!cache_initialized) {
            cacheInstance(root_instance, "");
            cache_initialized = true;
        }

        // Handle search
        if (strlen(search_filter) > 0) {
            if (!show_search_results) {
                search_results.clear();
                searchInstances(root_instance, search_filter);
                show_search_results = true;
            }

            // Display search results
            ImGui::Text("Search Results (%d):", (int)search_results.size());
            ImGui::Separator();

            for (const auto& instance : search_results) {
                if (instance.address == 0) continue;

                std::string display_name = getInstanceDisplayName(instance);
                bool is_selected = (selected_instance.address == instance.address);

                ImGui::PushID(instance.address);

                if (ImGui::Selectable(display_name.c_str(), is_selected)) {
                    selected_instance = instance;
                }

                // Handle double-click
                if (ImGui::IsItemClicked()) {
                    auto now = std::chrono::steady_clock::now();
                    if (last_clicked_item == std::to_string(instance.address) &&
                        (now - last_click_time) < double_click_duration) {
                        // Double-click - expand/collapse
                        if (expanded_instances.count(instance.address)) {
                            expanded_instances.erase(instance.address);
                        }
                        else {
                            expanded_instances.insert(instance.address);
                        }
                        last_clicked_item = "";
                    }
                    else {
                        selected_instance = instance;
                        last_clicked_item = std::to_string(instance.address);
                        last_click_time = now;
                    }
                }

                // Right-click context menu
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    selected_instance = instance;
                    ImGui::OpenPopup("InstanceContextMenu");
                }

                ImGui::PopID();
            }
        }
        else {
            show_search_results = false;
            // Display normal instance tree
            renderInstanceTree(root_instance, 0);
        }

    }
    catch (const std::exception& e) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", e.what());
    }

    ImGui::EndChild();

    // Right column - Instance properties
    ImGui::NextColumn();
    ImGui::BeginChild("InstanceProperties", ImVec2(0, 0), true);

    ImGui::Text("Instance Properties");
    ImGui::Separator();

    if (selected_instance.address != 0) {
        try {
            std::string name = getInstanceName(selected_instance);
            std::string class_name = getInstanceClassName(selected_instance);
            std::string path = getInstancePath(selected_instance);

            ImGui::Text("Name: %s", name.c_str());
            ImGui::Text("Class: %s", class_name.c_str());

            // Enhanced path display with copy buttons
            ImGui::Text("Path:");
            ImGui::SameLine();

            // Style the copy buttons to be more compact and attractive
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
            if (ImGui::Button("Copy Path##CopyPathBtn", ImVec2(80, 20))) {
                ImGui::SetClipboardText(path.c_str());
            }
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.6f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.3f, 0.7f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.1f, 0.5f, 1.0f));
            if (ImGui::Button("Copy Full Path##CopyFullPathBtn", ImVec2(120, 20))) {
                std::string full_path = getInstanceFullPath(selected_instance);
                ImGui::SetClipboardText(full_path.c_str());
            }
            ImGui::PopStyleColor(3);

            // Display the path with word wrapping
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x - 20);
            ImGui::TextWrapped("%s", path.c_str());
            ImGui::PopTextWrapPos();

            ImGui::Text("Address: 0x%llX", selected_instance.address);

            ImGui::Separator();

            // Show position if it's a BasePart
            if (class_name == "Part" || class_name == "BasePart" || class_name == "Model") {
                try {
                    math::Vector3 position = selected_instance.get_pos();
                    ImGui::Text("Position: X: %.2f, Y: %.2f, Z: %.2f", position.x, position.y, position.z);

                    if (ImGui::Button("Teleport To")) {
                        safe_teleport_to(position);
                    }
                }
                catch (...) {
                    ImGui::Text("Position: Not accessible");
                }
            }

            // Show other properties based on class
            if (class_name == "Part" || class_name == "BasePart") {
                try {
                    math::Vector3 size = selected_instance.get_part_size();
                    ImGui::Text("Size: X: %.2f, Y: %.2f, Z: %.2f", size.x, size.y, size.z);

                    bool can_collide = selected_instance.get_cancollide();
                    ImGui::Text("Can Collide: %s", can_collide ? "Yes" : "No");

                    if (ImGui::Button("Toggle Collision")) {
                        selected_instance.write_cancollide(!can_collide);
                    }
                }
                catch (...) {
                    ImGui::Text("Properties: Not accessible");
                }
            }

        }
        catch (...) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error reading instance properties");
        }
    }
    else {
        ImGui::Text("No instance selected");
    }

    ImGui::EndChild();

    // Context menu for instances
    if (ImGui::BeginPopup("InstanceContextMenu")) {
        if (ImGui::MenuItem("Copy Address")) {
            ImGui::SetClipboardText(std::to_string(selected_instance.address).c_str());
        }

        if (ImGui::MenuItem("Copy Name")) {
            std::string name = getInstanceName(selected_instance);
            ImGui::SetClipboardText(name.c_str());
        }

        if (ImGui::MenuItem("Copy Class")) {
            std::string class_name = getInstanceClassName(selected_instance);
            ImGui::SetClipboardText(class_name.c_str());
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Teleport To")) {
            try {
                math::Vector3 position = selected_instance.get_pos();
                if (position.x != 0 || position.y != 0 || position.z != 0) {
                    safe_teleport_to(position);
                }
            }
            catch (...) {
                // Ignore teleport errors
            }
        }

        if (ImGui::MenuItem("View Properties")) {
            // Could expand to show instance properties
        }

        if (ImGui::MenuItem("Copy Path")) {
            std::string path = getInstancePath(selected_instance);
            ImGui::SetClipboardText(path.c_str());
        }

        if (ImGui::MenuItem("Copy Full Path (with Classes)")) {
            std::string full_path = getInstanceFullPath(selected_instance);
            ImGui::SetClipboardText(full_path.c_str());
        }

        if (ImGui::MenuItem("Copy Path for Scripting")) {
            std::string path = getInstancePath(selected_instance);
            std::string script_path = "game." + path;
            ImGui::SetClipboardText(script_path.c_str());
        }

        if (ImGui::MenuItem("Find References")) {
            // Could implement reference finding
        }

        ImGui::EndPopup();
    }

    // Status bar with performance info
    ImGui::Separator();
    ImGui::Text("Selected: %s | Instances: %d | Expanded: %d | Cache: %d/%d",
        selected_instance.address == 0 ? "None" : getInstanceName(selected_instance).c_str(),
        (int)instance_cache.size(),
        (int)expanded_instances.size(),
        (int)instance_cache.size(),
        MAX_CACHE_SIZE);

    // Search mode indicator
    if (search_by_path) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " | Path Search: ON");
    }

    // Performance optimization indicator
    if (instance_cache.size() > MAX_CACHE_SIZE * 0.8) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Performance: Cache nearly full, consider refreshing");
    }
    else if (instance_cache.size() > MAX_CACHE_SIZE * 0.5) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance: Cache moderate size");
    }
    else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Performance: Cache optimal size");
    }

    ImGui::EndChild();
    ImGui::EndChild();
}





static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winhttp.lib")


bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

bool fullsc(HWND windowHandle);

void movewindow(HWND hw);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


static ConfigSystem g_config_system;

// Visual-only disabled scope: dims widgets but keeps them interactive
static std::vector<bool> g_visual_disabled_stack;
static inline void BeginVisualDisabled(bool disabled)
{
    (void)disabled;
    g_visual_disabled_stack.push_back(disabled);
}
static inline void EndVisualDisabled()
{
    if (!g_visual_disabled_stack.empty())
        g_visual_disabled_stack.pop_back();
}

// Configurable menu toggle keybind (defaults to Insert)
static keybind g_menu_toggle_keybind("menu_toggle_keybind");
static bool g_menu_custom_bind_enabled = false;
static const int g_menu_default_key = VK_END;

// Safer teleport utility: interpolate position via CFrame to reduce detections/kicks
static void safe_teleport_to(const math::Vector3& rawTarget)
{
    if (globals::handlingtp) return;

    instance hrp = globals::instances::lp.hrp;
    if (hrp.address == 0) return;

    math::Vector3 target = rawTarget;
    target.y += 0.5f; // small lift to avoid ground clipping

    CFrame currentCFrame = hrp.read_cframe();
    math::Vector3 start = currentCFrame.position;

    float dx = target.x - start.x;
    float dy = target.y - start.y;
    float dz = target.z - start.z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    int steps = (int)std::min(20.0f, std::max(1.0f, distance / 25.0f));
    for (int i = 1; i <= steps; ++i)
    {
        if (globals::handlingtp) return;
        float t = (float)i / (float)steps;
        math::Vector3 pos{ start.x + dx * t, start.y + dy * t, start.z + dz * t };
        currentCFrame.position = pos;
        hrp.write_cframe(currentCFrame);
        hrp.write_velocity({ 0.0f, 0.0f, 0.0f });
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }

    currentCFrame.position = target;
    hrp.write_cframe(currentCFrame);
    hrp.write_velocity({ 0.0f, 0.0f, 0.0f });
}

// Only show Target HUD when there is a valid cached target
static inline bool shouldTargetHudBeActive() {
    return (globals::instances::cachedtarget.head.address != 0) ||
        (globals::instances::cachedlasttarget.head.address != 0);
}

// Force rescan function
void force_rescan() {
    // Clear cached players to force a fresh scan
    globals::instances::cachedplayers.clear();

    // Reset game structure cache to force re-detection
    static bool* game_structure_cached = nullptr;
    if (game_structure_cached) {
        *game_structure_cached = false;
    }

    // Force immediate player cache refresh
    if (is_valid_address(globals::instances::datamodel.address)) {
        globals::instances::players = globals::instances::datamodel.read_service("Players");
    }

    // Clear target cache
    globals::instances::cachedtarget = {};
    globals::instances::cachedlasttarget = {};
}

// Waypoint rendering removed
void render_waypoints() {
    return;

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    auto view_matrix = globals::instances::visualengine.GetViewMatrix();
    math::Vector3 local_pos = globals::instances::lp.hrp.get_pos();

    for (auto& waypoint : waypoints) {
        if (!waypoint.visible) continue;

        // Convert 3D position to 2D screen coordinates
        math::Vector3 waypoint_pos = waypoint.position;
        math::Vector2 screen_pos = roblox::instance::worldtoscreen(waypoint_pos);

        if (screen_pos.x != -1.0f && screen_pos.y != -1.0f) {
            // Calculate distance
            float distance = (waypoint_pos - local_pos).magnitude();

            // Color for waypoint visuals
            ImU32 waypoint_color = IM_COL32(0, 255, 255, 255); // Turquoise

            // Draw 3D circle effect around the waypoint (like sonar)
            float circle_radius = 4.0f;
            const int num_segments = 64; // Increased segments for smoother circle
            std::vector<ImVec2> screen_points;
            screen_points.reserve(num_segments + 1);

            // Get screen dimensions for bounds checking
            auto screen_dimensions = globals::instances::visualengine.get_dimensions();

            // Create 3D circle points around the waypoint position
            for (int i = 0; i <= num_segments; i++) {
                float angle = (2.0f * M_PI * i) / num_segments;

                // Create 3D circle point around the waypoint
                math::Vector3 circle_point_3d(
                    waypoint_pos.x + cos(angle) * circle_radius,
                    waypoint_pos.y, // Keep at waypoint height
                    waypoint_pos.z + sin(angle) * circle_radius
                );

                math::Vector2 circle_screen_pos = roblox::instance::worldtoscreen(circle_point_3d);


                // More lenient bounds checking to maintain circle integrity
                if (circle_screen_pos.x != -1.0f && circle_screen_pos.y != -1.0f &&
                    std::isfinite(circle_screen_pos.x) && std::isfinite(circle_screen_pos.y) &&
                    circle_screen_pos.x > -2000 && circle_screen_pos.x < screen_dimensions.x + 2000 &&
                    circle_screen_pos.y > -2000 && circle_screen_pos.y < screen_dimensions.y + 2000) {
                    screen_points.push_back(ImVec2(circle_screen_pos.x, circle_screen_pos.y));
                }
            }

            // Draw the 3D circle by connecting all points with improved gap handling
            if (screen_points.size() >= 12) { // Increased minimum points for better circle
                for (size_t i = 0; i < screen_points.size() - 1; i++) {
                    // Calculate distance between consecutive points
                    float dx = screen_points[i + 1].x - screen_points[i].x;
                    float dy = screen_points[i + 1].y - screen_points[i].y;
                    if (distance < 800.0f) {
                        draw_list->AddLine(
                            screen_points[i],
                            screen_points[i + 1],
                            waypoint_color,
                            2.0f
                        );
                    }
                }

                // Close the circle with improved gap handling
                if (screen_points.size() > 2) {
                    float dx = screen_points.front().x - screen_points.back().x;
                    float dy = screen_points.front().y - screen_points.back().y;
                    float distance = sqrtf(dx * dx + dy * dy);

                    if (distance < 800.0f) {
                        draw_list->AddLine(
                            screen_points.back(),
                            screen_points.front(),
                            waypoint_color,
                            2.0f
                        );
                    }
                }
            }

            // Draw waypoint icon at the 3D circle center (outer ring + inner dot)
            {
                float base_radius = 6.0f;
                // Slight size attenuation with distance
                float scaled = base_radius * (1.0f - std::min(distance, 500.0f) / 1000.0f);
                float r = std::max(3.0f, scaled);
                ImU32 ring_col = ImGui::GetColorU32(ImGuiCol_SliderGrab);
                ImU32 dot_col = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
                draw_list->AddCircle(ImVec2(screen_pos.x, screen_pos.y), r, ring_col, 24, 2.0f);
                draw_list->AddCircleFilled(ImVec2(screen_pos.x, screen_pos.y), r * 0.45f, dot_col, 24);
            }

            // Draw distance near the base of the 3D line
            std::string distance_text = "[" + std::to_string((int)distance) + "m]";
            ImVec2 distance_pos(screen_pos.x - 20, screen_pos.y + 20);
            draw_list->AddText(distance_pos, waypoint_color, distance_text.c_str());
        }
    }
}

void render_desync_timer() {
    if (!globals::misc::targethud) return; // Only show when HUD is enabled
    if (!globals::misc::desync_active) return; // Only show when desync is active

    // Create a compact timer UI similar to the image
    static ImVec2 timer_pos = ImVec2(static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.0f - 100.0f, 50.0f);

    ImGui::SetNextWindowPos(timer_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 60), ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;

    ImGui::Begin("DesyncTimer", nullptr, window_flags);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    // Dark gray background with red border
    ImU32 bg_color = IM_COL32(47, 48, 46, 255); // Dark gray
    ImU32 border_color = IM_COL32(64, 128, 255, 255); // Red border
    ImU32 text_color = IM_COL32(255, 255, 255, 255); // White text
    ImU32 text_outline_color = IM_COL32(0, 0, 0, 255); // Black outline for text

    // Draw background
    draw->AddRectFilled(window_pos, window_pos + window_size, bg_color, 6.0f);

    // Draw red outline around the entire rectangle
    draw->AddRect(window_pos, window_pos + window_size, border_color, 6.0f, 0, 3.0f);

    // Calculate progress (0.0 to 1.0)
    float progress = globals::misc::desync_timer / globals::misc::desync_max_time;
    progress = std::min(progress, 1.0f);

    // Draw progress bar (squared corners)
    ImVec2 bar_start = ImVec2(window_pos.x + 10, window_pos.y + 35);
    ImVec2 bar_end = ImVec2(window_pos.x + window_size.x - 10, window_pos.y + 45);
    ImVec2 bar_fill_end = ImVec2(bar_start.x + (bar_end.x - bar_start.x) * progress, bar_end.y);

    // Progress bar background (squared)
    draw->AddRectFilled(bar_start, bar_end, IM_COL32(60, 60, 60, 255), 0.0f);
    // Progress bar fill - RED (squared)
    if (progress > 0.0f) {
        draw->AddRectFilled(bar_start, bar_fill_end, IM_COL32(64, 128, 255, 255), 0.0f);
    }

    // Text: "desync : false : 0.0s"
    std::string status_text = "desync : " + std::string(globals::misc::desync_active ? "true" : "false") + " : " +
        std::to_string(static_cast<int>(globals::misc::desync_timer)) + "." +
        std::to_string(static_cast<int>((globals::misc::desync_timer - static_cast<int>(globals::misc::desync_timer)) * 10)) + "s";

    // Center the text horizontally
    ImVec2 text_size = ImGui::CalcTextSize(status_text.c_str());
    ImVec2 text_pos = ImVec2(window_pos.x + (window_size.x - text_size.x) / 2.0f, window_pos.y + 10);

    // Draw text with outline (draw outline first, then text on top)
    draw->AddText(ImVec2(text_pos.x - 1, text_pos.y - 1), text_outline_color, status_text.c_str());
    draw->AddText(ImVec2(text_pos.x + 1, text_pos.y - 1), text_outline_color, status_text.c_str());
    draw->AddText(ImVec2(text_pos.x - 1, text_pos.y + 1), text_outline_color, status_text.c_str());
    draw->AddText(ImVec2(text_pos.x + 1, text_pos.y + 1), text_outline_color, status_text.c_str());
    draw->AddText(text_pos, text_color, status_text.c_str());

    ImGui::End();
}

void render_player_list() {
    if (!globals::misc::playerlist) return;

    if (!overlay::get_avatar_manager()) {
        overlay::initialize_avatar_system();
    }

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;

    static ImVec2 playerlist_pos = ImVec2(500, 10);
    static bool first_time = true;
    static int selected_player = -1;
    static float side_panel_animation = 0.0f;
    static std::vector<std::string> status_options = { "Enemy", "Friendly", "Neutral", "Client" };
    static std::vector<int> player_status;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize;

    if (!overlay::visible) {
        window_flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
    }

    if (first_time || !overlay::visible) {
        ImGui::SetNextWindowPos(playerlist_pos, ImGuiCond_Always);
        first_time = false;
    }

    std::vector<roblox::player> players;
    if (globals::instances::cachedplayers.size() > 0) {
        players = globals::instances::cachedplayers;
    }

    if (player_status.size() != players.size()) {
        player_status.resize(players.size(), 2);

        for (size_t i = 0; i < players.size(); i++) {
            auto& player = players[i];

            if (player.name == globals::instances::lp.name) {
                player_status[i] = 3;
            }
            else if (std::find(globals::instances::whitelist.begin(), globals::instances::whitelist.end(), player.name) != globals::instances::whitelist.end()) {
                player_status[i] = 1;
            }
            else if (std::find(globals::instances::blacklist.begin(), globals::instances::blacklist.end(), player.name) != globals::instances::blacklist.end()) {
                player_status[i] = 0;
            }
        }
    }

    auto isWhitelisted = [](const std::string& name) {
        return std::find(globals::instances::whitelist.begin(), globals::instances::whitelist.end(), name) != globals::instances::whitelist.end();
        };

    auto isBlacklisted = [](const std::string& name) {
        return std::find(globals::instances::blacklist.begin(), globals::instances::blacklist.end(), name) != globals::instances::blacklist.end();
        };

    auto addToWhitelist = [](const std::string& name) {
        if (std::find(globals::instances::whitelist.begin(), globals::instances::whitelist.end(), name) == globals::instances::whitelist.end()) {
            globals::instances::whitelist.push_back(name);
        }
        auto it = std::find(globals::instances::blacklist.begin(), globals::instances::blacklist.end(), name);
        if (it != globals::instances::blacklist.end()) {
            globals::instances::blacklist.erase(it);
        }
        };

    auto addToBlacklist = [](const std::string& name) {
        if (std::find(globals::instances::blacklist.begin(), globals::instances::blacklist.end(), name) == globals::instances::blacklist.end()) {
            globals::instances::blacklist.push_back(name);
        }
        auto it = std::find(globals::instances::whitelist.begin(), globals::instances::whitelist.end(), name);
        if (it != globals::instances::whitelist.end()) {
            globals::instances::whitelist.erase(it);
        }
        };

    auto removeFromLists = [](const std::string& name) {
        auto it = std::find(globals::instances::whitelist.begin(), globals::instances::whitelist.end(), name);
        if (it != globals::instances::whitelist.end()) {
            globals::instances::whitelist.erase(it);
        }
        it = std::find(globals::instances::blacklist.begin(), globals::instances::blacklist.end(), name);
        if (it != globals::instances::blacklist.end()) {
            globals::instances::blacklist.erase(it);
        }
        };

    float content_width = 300.0f;
    float padding = 8.0f;
    float header_height = 30.0f;
    float player_item_height = 45.0f;
    float max_height = 500.0f;
    float side_panel_width = 350.0f;

    float target_animation = (selected_player >= 0) ? 1.0f : 0.0f;
    side_panel_animation += (target_animation - side_panel_animation) * 0.15f;
    float animated_side_width = side_panel_width * side_panel_animation;

    float total_width = content_width + (padding * 2);
    float total_height = header_height + max_height + padding;

    ImGui::SetNextWindowSize(ImVec2(total_width + animated_side_width, total_height), ImGuiCond_Always);
    ImGui::Begin("PlayerList", nullptr, window_flags);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();

    if (overlay::visible) {
        playerlist_pos = window_pos;
    }
    auto* avatar_mgr = overlay::get_avatar_manager();
    ImU32 text_color = IM_COL32(255, 255, 255, 255);
    ImU32 enemy_color = IM_COL32(255, 100, 100, 255);
    ImU32 friendly_color = IM_COL32(100, 255, 100, 255);
    ImU32 neutral_color = IM_COL32(200, 200, 200, 255);
    ImU32 client_color = IM_COL32(100, 150, 255, 255);
    ImU32 top_line_color = ImGui::GetColorU32(ImGuiCol_SliderGrab);

    draw->AddRectFilled(window_pos, ImVec2(window_pos.x + total_width, window_pos.y + 2), top_line_color, 0.0f);
    draw->AddText(ImVec2(window_pos.x + padding, window_pos.y + 8), text_color, "Players");

    ImGui::SetCursorPos(ImVec2(padding, header_height));

    if (ImGui::BeginChild("PlayerList", ImVec2(total_width - padding * 2, max_height), true)) {
        if (players.empty()) {
            ImGui::Text("No players found");
        }
        else {
            for (size_t i = 0; i < players.size(); i++) {
                auto& player = players[i];
                ImGui::PushID(static_cast<int>(i));

                bool is_selected = (selected_player == static_cast<int>(i));
                bool is_client = (player.name == globals::instances::lp.name);

                if (is_selected) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.41f, 0.0f, 1.0f, 0.4f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.41f, 0.0f, 1.0f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.41f, 0.0f, 1.0f, 0.6f));
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

                if (ImGui::Button("##player", ImVec2(-1, player_item_height))) {
                    if (!is_client) {
                        selected_player = (selected_player == static_cast<int>(i)) ? -1 : static_cast<int>(i);
                    }
                }

                ImGui::PopStyleVar(3);
                ImGui::PopStyleColor(3);

                ImVec2 button_min = ImGui::GetItemRectMin();
                ImDrawList* window_draw_list = ImGui::GetWindowDrawList();

                ImVec2 avatar_start = ImVec2(button_min.x + 3, button_min.y + 3);
                ImVec2 avatar_end = ImVec2(avatar_start.x + 39, avatar_start.y + 39);

                ImTextureID avatar_texture = avatar_mgr->getAvatarTexture(player.userid.address);
                ImVec2 avatar_size2 = ImVec2(36, 36);

                window_draw_list->AddRectFilled(avatar_start, avatar_end, IM_COL32(40, 40, 40, 255), 4.0f);
                window_draw_list->AddRect(avatar_start, avatar_end, IM_COL32(80, 80, 80, 255), 4.0f);
                ImVec2 center_offset = ImVec2(
                    (avatar_end.x - avatar_start.x - avatar_size2.x) / 2.0f,
                    (avatar_end.y - avatar_start.y - avatar_size2.y) / 2.0f
                );
                ImGui::SetCursorScreenPos(avatar_start);

                ImVec2 image_pos = ImVec2(avatar_start.x + center_offset.x, avatar_start.y + center_offset.y);

                ImGui::SetCursorScreenPos(image_pos);
                if (avatar_texture) {
                    ImGui::Image(avatar_texture, avatar_size2);
                }
                else {
                    window_draw_list->AddText(ImVec2(avatar_start.x + 19.5f - ImGui::CalcTextSize("IMG").x / 2,
                        avatar_start.y + 19.5f - ImGui::CalcTextSize("IMG").y / 2), IM_COL32(120, 120, 120, 255), "IMG");
                }

                float info_x = avatar_end.x + 8;
                std::string display_name = player.name.length() > 20 ? player.name.substr(0, 17) + "..." : player.name;
                window_draw_list->AddText(ImVec2(info_x, button_min.y + 5), text_color, display_name.c_str());

                ImU32 status_color = neutral_color;
                std::string status_text = "Neutral";

                if (is_client) {
                    player_status[i] = 3;
                    status_color = client_color;
                    status_text = "Client";
                }
                else if (i < player_status.size()) {
                    switch (player_status[i]) {
                    case 0: status_color = enemy_color; status_text = "Enemy"; break;
                    case 1: status_color = friendly_color; status_text = "Friendly"; break;
                    case 2: status_color = neutral_color; status_text = "Neutral"; break;
                    case 3: status_color = client_color; status_text = "Client"; break;
                    }
                }

                window_draw_list->AddText(ImVec2(info_x, button_min.y + 17), status_color, status_text.c_str());
                window_draw_list->AddText(ImVec2(info_x, button_min.y + 29), IM_COL32(180, 180, 180, 255),
                    ("ID: " + std::to_string(player.userid.address)).c_str());

                ImGui::PopID();
            }
        }
    }
    ImGui::EndChild();

    static bool spectating = false;
    if (side_panel_animation > 0.01f && selected_player >= 0 && selected_player < static_cast<int>(players.size())) {
        auto& selected = players[selected_player];
        bool is_client = (selected.name == globals::instances::lp.name);

        float panel_x = window_pos.x + total_width;
        float panel_y = window_pos.y;

        draw->AddRectFilled(ImVec2(panel_x, panel_y), ImVec2(panel_x + animated_side_width, panel_y + total_height),
            IM_COL32(15, 15, 15, 200), 0.0f);
        draw->AddRect(ImVec2(panel_x, panel_y), ImVec2(panel_x + animated_side_width, panel_y + total_height),
            IM_COL32(60, 60, 60, 255), 0.0f);
        draw->AddRectFilled(ImVec2(panel_x, panel_y), ImVec2(panel_x + animated_side_width, panel_y + 2),
            top_line_color, 0.0f);

        if (side_panel_animation > 0.8f && !is_client) {
            float pad = 20.0f;
            float y = panel_y + 25.0f;

            if (avatar_mgr) {
                ImTextureID avatar_texture = avatar_mgr->getAvatarTexture(selected.userid.address);

                ImVec2 avatar_start = ImVec2(panel_x + pad, y);
                ImVec2 avatar_size = ImVec2(150, 150);

                ImGui::SetCursorScreenPos(avatar_start);

                if (avatar_texture) {
                    ImGui::Image(avatar_texture, avatar_size);
                }
                else {
                    std::string userId = std::to_string(selected.userid.address);
                    AvatarState state = avatar_mgr->getAvatarState(userId);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.09f, 0.09f, 0.09f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

                    switch (state) {
                    case AvatarState::NotRequested:
                        if (ImGui::Button("Load Avatar", avatar_size)) {
                            avatar_mgr->requestAvatar(selected.userid.address);
                        }
                        break;
                    case AvatarState::Downloading:
                        ImGui::Button("Loading...", avatar_size);
                        break;
                    case AvatarState::Failed:
                        if (ImGui::Button("Retry Avatar", avatar_size)) {
                            avatar_mgr->requestAvatar(selected.userid.address);
                        }
                        break;
                    default:
                        ImGui::Button("Avatar", avatar_size);
                        break;
                    }

                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                }
            }
            else {
                draw->AddRectFilled(ImVec2(panel_x + pad, y), ImVec2(panel_x + pad + 150, y + 150),
                    IM_COL32(40, 40, 40, 255), 6.0f);
                draw->AddRect(ImVec2(panel_x + pad, y), ImVec2(panel_x + pad + 150, y + 150),
                    IM_COL32(80, 80, 80, 255), 6.0f);
                draw->AddText(ImVec2(panel_x + pad + 75 - ImGui::CalcTextSize("AVATAR").x / 2,
                    y + 75 - ImGui::CalcTextSize("AVATAR").y / 2),
                    IM_COL32(120, 120, 120, 255), "AVATAR");
            }

            float info_x = panel_x + pad + 150 + 15.0f;
            draw->AddText(ImVec2(info_x, y), text_color, selected.name.c_str());
            draw->AddText(ImVec2(info_x, y + 25), IM_COL32(180, 180, 180, 255), "Position:");
            Vector3 pos = selected.hrp.get_pos();

            int health = selected.humanoid.read_health();
            int maxhealth = selected.humanoid.read_maxhealth();

            auto x = std::to_string(static_cast<int>(pos.x));
            std::string yz = std::to_string(static_cast<int>(pos.y));
            auto z = std::to_string(static_cast<int>(pos.z));
            std::string posz = "X: " + x + ", Y: " + yz + ", Z: " + z;
            std::string healthz = "Health: " + std::to_string(health) + "/" + std::to_string(maxhealth);
            draw->AddText(ImVec2(info_x, y + 43), text_color, posz.c_str());
            draw->AddText(ImVec2(info_x, y + 68), text_color, healthz.c_str());

            std::string status_display = "Status: " +
                (selected_player < static_cast<int>(player_status.size()) ?
                    status_options[player_status[selected_player]] : "Neutral");
            draw->AddText(ImVec2(info_x, y + 93), text_color, status_display.c_str());

            float btn_y = y + 150 + 30;
            float btn_w = (animated_side_width - pad * 3) * 0.5f;
            float btn_h = 22.0f;
            float btn_spacing = 6.0f;

            ImVec2 mouse_pos = ImGui::GetMousePos();
            bool mouse_clicked = ImGui::IsMouseClicked(0);

            auto draw_button = [&](ImVec2 pos, ImVec2 size, const char* text, bool& clicked) {
                bool hovered = mouse_pos.x >= pos.x && mouse_pos.x <= pos.x + size.x &&
                    mouse_pos.y >= pos.y && mouse_pos.y <= pos.y + size.y;

                ImU32 btn_color = IM_COL32(23, 23, 23, 255);
                if (hovered) {
                    btn_color = IM_COL32(38, 38, 38, 255);
                    if (mouse_clicked) {
                        clicked = true;
                        btn_color = IM_COL32(51, 51, 51, 255);
                    }
                }

                draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), btn_color, 2.0f);
                draw->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(80, 80, 80, 255), 2.0f);

                ImVec2 text_size = ImGui::CalcTextSize(text);
                ImVec2 text_pos = ImVec2(pos.x + size.x * 0.5f - text_size.x * 0.5f,
                    pos.y + size.y * 0.5f - text_size.y * 0.5f);
                draw->AddText(text_pos, text_color, text);
                };

            bool spectate_clicked = false, kill_clicked = false, auto_kill_clicked = false,
                bring_clicked = false, teleport_clicked = false;
            static std::string cool = "Spectate";
            if (spectating)
                cool = "Unspectate";
            else
                cool = "Spectate";

            draw_button(ImVec2(panel_x + pad, btn_y), ImVec2(btn_w, btn_h), cool.c_str(), spectate_clicked);
            draw_button(ImVec2(panel_x + pad + btn_w + pad, btn_y), ImVec2(btn_w, btn_h), "Kill", kill_clicked);

            draw_button(ImVec2(panel_x + pad, btn_y + btn_h + btn_spacing), ImVec2(btn_w, btn_h), "Auto Kill", auto_kill_clicked);
            draw_button(ImVec2(panel_x + pad + btn_w + pad, btn_y + btn_h + btn_spacing), ImVec2(btn_w, btn_h), "Bring", bring_clicked);

            draw_button(ImVec2(panel_x + pad, btn_y + (btn_h + btn_spacing) * 2),
                ImVec2(animated_side_width - pad * 2, btn_h), "Teleport", teleport_clicked);

            float combo_y = btn_y + (btn_h + btn_spacing) * 3;
            ImVec2 combo_pos = ImVec2(panel_x + pad, combo_y);
            ImVec2 combo_size = ImVec2(animated_side_width - pad * 2, btn_h);

            bool combo_hovered = mouse_pos.x >= combo_pos.x && mouse_pos.x <= combo_pos.x + combo_size.x &&
                mouse_pos.y >= combo_pos.y && mouse_pos.y <= combo_pos.y + combo_size.y;

            static bool combo_open = false;
            if (combo_hovered && mouse_clicked) {
                combo_open = !combo_open;
            }

            ImU32 combo_color = combo_hovered ? IM_COL32(38, 38, 38, 255) : IM_COL32(23, 23, 23, 255);
            draw->AddRectFilled(combo_pos, ImVec2(combo_pos.x + combo_size.x, combo_pos.y + combo_size.y), combo_color, 2.0f);
            draw->AddRect(combo_pos, ImVec2(combo_pos.x + combo_size.x, combo_pos.y + combo_size.y), IM_COL32(80, 80, 80, 255), 2.0f);

            std::string combo_text = selected_player < static_cast<int>(player_status.size()) ?
                status_options[player_status[selected_player]] : "Neutral";
            ImVec2 combo_text_pos = ImVec2(combo_pos.x + 8, combo_pos.y + combo_size.y * 0.5f - ImGui::CalcTextSize(combo_text.c_str()).y * 0.5f);
            draw->AddText(combo_text_pos, text_color, combo_text.c_str());

            draw->AddText(ImVec2(combo_pos.x + combo_size.x - 15, combo_text_pos.y), text_color, "v");

            if (combo_open) {
                float dropdown_y = combo_pos.y + combo_size.y + 2;
                for (int i = 0; i < 3; i++) {
                    ImVec2 item_pos = ImVec2(combo_pos.x, dropdown_y + i * btn_h);
                    bool item_hovered = mouse_pos.x >= item_pos.x && mouse_pos.x <= item_pos.x + combo_size.x &&
                        mouse_pos.y >= item_pos.y && mouse_pos.y <= item_pos.y + btn_h;

                    ImU32 item_color = item_hovered ? IM_COL32(38, 38, 38, 255) : IM_COL32(23, 23, 23, 255);
                    draw->AddRectFilled(item_pos, ImVec2(item_pos.x + combo_size.x, item_pos.y + btn_h), item_color, 0.0f);
                    draw->AddRect(item_pos, ImVec2(item_pos.x + combo_size.x, item_pos.y + btn_h), IM_COL32(80, 80, 80, 255), 0.0f);

                    ImVec2 item_text_pos = ImVec2(item_pos.x + 8, item_pos.y + btn_h * 0.5f - ImGui::CalcTextSize(status_options[i].c_str()).y * 0.5f);
                    draw->AddText(item_text_pos, text_color, status_options[i].c_str());

                    if (item_hovered && mouse_clicked && selected_player >= 0 && selected_player < static_cast<int>(player_status.size())) {
                        int old_status = player_status[selected_player];
                        player_status[selected_player] = i;

                        if (i == 0) {
                            addToBlacklist(selected.name);
                        }
                        else if (i == 1) {
                            addToWhitelist(selected.name);
                        }
                        else if (i == 2) {
                            removeFromLists(selected.name);
                        }

                        combo_open = false;
                    }
                }
            }

            if (mouse_clicked && !combo_hovered && combo_open) {
                combo_open = false;
            }

            if (spectate_clicked) {
                roblox::instance cam;
                if (!spectating) {
                    spectating = true;
                    cam.spectate(selected.hrp.address);
                }
                else {
                    spectating = false;
                    cam.unspectate();
                }
            }
            if (kill_clicked) {
                globals::bools::name = selected.name;
                globals::bools::entity = selected;
                globals::bools::kill = true;
            }
            if (auto_kill_clicked) {
                globals::bools::name = selected.name;
                globals::bools::entity = selected;
                globals::bools::autokill = true;
            }
            if (bring_clicked) {
                globals::bools::name = selected.name;
                globals::bools::entity = selected;
                globals::bools::bring = true;
            }
            if (teleport_clicked) {
                globals::instances::lp.hrp.write_position(selected.hrp.get_pos());
            }
        }
        else if (is_client && side_panel_animation > 0.8f) {
            float pad = 20.0f;
            float info_x = panel_x + pad;
            float y = panel_y + 25.0f;

            draw->AddText(ImVec2(info_x, y), client_color, "This is you!");
            draw->AddText(ImVec2(info_x, y + 25), text_color, selected.name.c_str());
            draw->AddText(ImVec2(info_x, y + 50), IM_COL32(180, 180, 180, 255), "Status: Client");
        }
    }

    ImGui::End();
}

void render_target_hud() {
    if (!globals::misc::targethud) return;
    // Only show while an active target is locked
    if (globals::instances::cachedtarget.head.address == 0) return;

    roblox::player target = globals::instances::cachedtarget;
    if (target.name.empty()) return;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;

    static ImVec2 targethudpos = ImVec2(static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.0f - 90.0f, static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.0f + 120.0f);
    static bool first_time = true;
    static bool isDragging = false;
    static ImVec2 dragDelta;
    static float animatedHealth = 100.0f;
    static int lastHealth = 100;
    static float animationTimer = 0.0f;
    static float healthTextAlpha = 0.0f;
    static float healthTextTargetAlpha = 0.0f;
    static float healthTextLerpSpeed = 3.0f;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground;

    if (!overlay::visible) {
        window_flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
    }

    if (first_time || !overlay::visible) {
        ImGui::SetNextWindowPos(targethudpos, ImGuiCond_Always);
        first_time = false;
    }

    int health = target.humanoid.read_health();
    int maxHealth = target.humanoid.read_maxhealth();

    if (maxHealth <= 0) maxHealth = 100;
    if (health < 0) health = 0;

    if (lastHealth != health) {
        if (health < lastHealth) {
            animationTimer = 1.0f;
            // Show health text when player gets hit (only if not at full health)
            if (health < maxHealth) {
                healthTextTargetAlpha = 1.0f;
            }
        }
        lastHealth = health;
    }

    // Hide health text when health is at full health
    if (health >= maxHealth) {
        healthTextTargetAlpha = 0.0f;
    }

    // Smooth lerp animation for health text alpha
    float deltaTime = ImGui::GetIO().DeltaTime;
    if (std::abs(healthTextAlpha - healthTextTargetAlpha) > 0.01f) {
        healthTextAlpha += (healthTextTargetAlpha - healthTextAlpha) * healthTextLerpSpeed * deltaTime;
    }
    else {
        healthTextAlpha = healthTextTargetAlpha;
    }

    float targetHealthPercentage = std::clamp(static_cast<float>(health) / maxHealth, 0.0f, 1.0f);
    float currentHealthPercentage = std::clamp(animatedHealth / maxHealth, 0.0f, 1.0f);

    if (animationTimer > 0.0f) {
        animationTimer = std::max(0.0f, animationTimer - ImGui::GetIO().DeltaTime);
    }

    if (std::abs(currentHealthPercentage - targetHealthPercentage) > 0.001f) {
        float animationSpeed = 1.0f * ImGui::GetIO().DeltaTime;
        if (targetHealthPercentage < currentHealthPercentage) {
            currentHealthPercentage = std::max(targetHealthPercentage, currentHealthPercentage - animationSpeed);
        }
        else {
            currentHealthPercentage = std::min(targetHealthPercentage, currentHealthPercentage + animationSpeed);
        }
        animatedHealth = currentHealthPercentage * maxHealth;
    }
    else {
        currentHealthPercentage = targetHealthPercentage;
    }

    const float PADDING = 5;
    // Compact card size to match the provided design
    float totalHeight = 62;
    ImVec2 windowSize(220, totalHeight);

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::Begin("TargetHUD", nullptr, window_flags);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();

    if (overlay::visible) {
        targethudpos = window_pos;
    }

    ImVec2 mousePos = ImGui::GetIO().MousePos;
    ImRect headerRect(targethudpos, targethudpos + windowSize);
    // Colors for the compact card
    const float card_rounding = 6.0f;
    ImU32 outer_col = IM_COL32(47, 48, 46, 255);
    ImU32 inner_col = IM_COL32(21, 24, 21, 255);
    ImU32 accent_color = IM_COL32(100, 160, 255, 255); // blue accent

    ImU32 healthBarColor = accent_color;
    if (animationTimer > 0.0f && health < lastHealth) {
        float flashIntensity = std::min(1.0f, animationTimer * 2.0f);
        healthBarColor = IM_COL32(
            static_cast<int>(105 + (150 * flashIntensity)),
            static_cast<int>(0 + (50 * flashIntensity)),
            static_cast<int>(255 - (100 * flashIntensity)),
            255
        );
    }

    if (ImGui::IsMouseClicked(0) && headerRect.Contains(mousePos) && overlay::visible) {
        isDragging = true;
        dragDelta = mousePos - targethudpos;
    }
    if (isDragging && ImGui::IsMouseDown(0)) {
        targethudpos = mousePos - dragDelta;
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        targethudpos.x = ImClamp(targethudpos.x, 0.0f, screenSize.x - windowSize.x);
        targethudpos.y = ImClamp(targethudpos.y, 0.0f, screenSize.y - totalHeight);
    }
    else {
        isDragging = false;
    }

    draw->AddRectFilled(
        targethudpos,
        targethudpos + windowSize,
        outer_col,
        card_rounding
    );
    draw->AddRectFilled(
        targethudpos + ImVec2(2, 2),
        targethudpos + windowSize - ImVec2(2, 2),
        inner_col,
        card_rounding
    );

    int barwidth = static_cast<int>(windowSize.x - 65);
    int healthbarwidth = static_cast<int>(currentHealthPercentage * barwidth);

    const float bar_height = 12.0f;
    ImVec2 healthBarBg_start = ImVec2(targethudpos.x + 55, targethudpos.y + 40);
    ImVec2 healthBarBg_end = ImVec2(targethudpos.x + 55 + barwidth, targethudpos.y + 40 + bar_height);

    ImU32 bar_bg = IM_COL32(60, 60, 60, 255);
    ImU32 bar_outline = IM_COL32(0, 0, 0, 160);
    ImU32 bar_fill = IM_COL32(100, 160, 255, 255); // blue

    draw->AddRectFilled(
        healthBarBg_start,
        healthBarBg_end,
        bar_bg,
        4.0f
    );

    if (healthbarwidth > 0) {
        draw->AddRectFilled(
            healthBarBg_start,
            ImVec2(targethudpos.x + 55 + healthbarwidth, healthBarBg_end.y),
            bar_fill,
            4.0f
        );
    }

    draw->AddRect(
        healthBarBg_start,
        healthBarBg_end,
        bar_outline,
        4.0f
    );

    // Use animated health for the text to follow the health bar animation
    int displayHealth = static_cast<int>(animatedHealth);
    std::string healthText = std::to_string(displayHealth) + " / " + std::to_string(maxHealth);
    // Health text positioned below the health bar (like in the image)
    ImVec2 hp_text_pos = ImVec2(targethudpos.x + 55, targethudpos.y + 52);

    auto* avatar_mgr = overlay::get_avatar_manager();
    if (avatar_mgr) {
        ImTextureID avatar_texture = avatar_mgr->getAvatarTexture(target.userid.address);

        if (avatar_texture) {
            draw->AddImage(
                avatar_texture,
                targethudpos + ImVec2(5, 5),
                targethudpos + ImVec2(45, 45)
            );
        }
        else {
            draw->AddRectFilled(
                targethudpos + ImVec2(5, 5),
                targethudpos + ImVec2(45, 45),
                IM_COL32(40, 40, 40, 255),
                2.0f
            );
            draw->AddText(
                targethudpos + ImVec2(25 - ImGui::CalcTextSize("IMG").x / 2, 25 - ImGui::CalcTextSize("IMG").y / 2),
                IM_COL32(120, 120, 120, 255),
                "IMG"
            );
        }
    }
    else {
        draw->AddRectFilled(
            targethudpos + ImVec2(5, 5),
            targethudpos + ImVec2(45, 45),
            IM_COL32(40, 40, 40, 255),
            2.0f
        );
        draw->AddText(
            targethudpos + ImVec2(25 - ImGui::CalcTextSize("IMG").x / 2, 25 - ImGui::CalcTextSize("IMG").y / 2),
            IM_COL32(120, 120, 120, 255),
            "IMG"
        );
    }

    // Only render health text if alpha is greater than 0
    if (healthTextAlpha > 0.01f) {
        // Red health text with shadow (like in the image)
        ImU32 shadowColor = IM_COL32(0, 0, 0, static_cast<int>(180 * healthTextAlpha));
        ImU32 textColor = IM_COL32(255, 0, 0, static_cast<int>(255 * healthTextAlpha)); // Red color

        // Draw shadow
        draw->AddText(
            hp_text_pos + ImVec2(1, 1),
            shadowColor,
            healthText.c_str()
        );

        // Draw main text
        draw->AddText(
            hp_text_pos,
            textColor,
            healthText.c_str()
        );
    }

    std::string display_name = target.name.length() > 16 ? target.name.substr(0, 13) + "..." : target.name;
    const char* status = (health > 0) ? "Alive" : "Dead";
    std::string header_text = display_name + " | " + status;
    draw->AddText(
        ImVec2(targethudpos.x + 55, targethudpos.y + 8),
        IM_COL32(255, 255, 255, 255),
        header_text.c_str()
    );

    ImGui::End();
}



void overlay::initialize_avatar_system() {
    if (g_pd3dDevice && !avatar_manager) {
        avatar_manager = std::make_unique<AvatarManager>(g_pd3dDevice, g_pd3dDeviceContext);

    }
}

void overlay::update_avatars() {
    // Performance optimization: Reduce avatar update frequency
    static auto last_update = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update);

    // Only update avatars every 2 seconds instead of every frame
    if (elapsed.count() < 2000) {
        return;
    }

    if (avatar_manager) {
        avatar_manager->update();
    }

    last_update = now;
}

AvatarManager* overlay::get_avatar_manager() {
    return avatar_manager.get();
}

void overlay::cleanup_avatar_system() {
    if (avatar_manager) {
        avatar_manager.reset();
    }
}


static ImFont* g_panelFont = nullptr; // Smooth font for panel-only text

// Waypoint panel removed
void render_waypoint_panel() {
    return;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize;

    if (!overlay::visible) {
        window_flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
    }

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Waypoints", &show_waypoint_panel, window_flags);
    if (g_panelFont) ImGui::PushFont(g_panelFont);
    // Smaller controls in this panel
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
    ImGui::SetWindowFontScale(0.90f);
    // Lazy-load waypoints from disk once when panel opens
    {
        static bool loaded_once = false;
        if (!loaded_once) {
            g_config_system.load_waypoints_files();
            loaded_once = true;
        }
    }

    // Header
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    ImU32 bg_color = IM_COL32(15, 15, 15, 200);
    ImU32 text_color = IM_COL32(255, 255, 255, 255);
    ImU32 top_line_color = ImGui::GetColorU32(ImGuiCol_SliderGrab);

    draw->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + 2.0f), top_line_color, 0.0f);
    draw->AddText(ImVec2(window_pos.x + 10.0f, window_pos.y + 8.0f), text_color, "Waypoints");

    ImGui::SetCursorPos(ImVec2(10.0f, 30.0f));

    ImGui::Separator();

    // Add new waypoint section
    ImGui::Text("Add New Waypoint:");
    ImGui::InputTextWithHint("##waypointname", "Waypoint name...", new_waypoint_name, sizeof(new_waypoint_name));

    // Button on next line to ensure it fits within the panel
    if (ImGui::Button("Add Current Position", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
        if (strlen(new_waypoint_name) > 0) {
            math::Vector3 current_pos = globals::instances::lp.hrp.get_pos();
            waypoints.emplace_back(std::string(new_waypoint_name), current_pos);
            memset(new_waypoint_name, 0, sizeof(new_waypoint_name));
            g_config_system.save_waypoints_files();
        }
    }

    ImGui::Separator();

    // Waypoint list
    ImGui::Text("Saved Waypoints (%d):", (int)waypoints.size());

    if (waypoints.empty()) {
        ImGui::TextDisabled("No waypoints saved");
    }
    else {
        for (size_t i = 0; i < waypoints.size(); i++) {
            auto& waypoint = waypoints[i];
            ImGui::PushID(static_cast<int>(i));

            // Icon + Waypoint name and visibility toggle
            {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 icon_pos = ImGui::GetCursorScreenPos();
                float outer_r = 6.0f; // icon radius
                ImU32 accent = ImGui::GetColorU32(ImGuiCol_SliderGrab);
                ImU32 accent_fill = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
                // Outer ring
                dl->AddCircle(ImVec2(icon_pos.x + outer_r, icon_pos.y + outer_r + 2.0f), outer_r, accent, 20, 1.5f);
                // Inner dot
                dl->AddCircleFilled(ImVec2(icon_pos.x + outer_r, icon_pos.y + outer_r + 2.0f), outer_r * 0.45f, accent_fill, 20);
                // Advance cursor to leave space for icon
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + outer_r * 2.0f + 8.0f);
            }

            ImGui::Checkbox(("##visible" + std::to_string(i)).c_str(), &waypoint.visible);
            ImGui::SameLine();
            ImGui::Text("%s", waypoint.name.c_str());

            // Position info
            ImGui::SameLine();
            ImGui::TextDisabled("(%.1f, %.1f, %.1f)", waypoint.position.x, waypoint.position.y, waypoint.position.z);

            // Action buttons
            if (ImGui::Button("Teleport")) {
                safe_teleport_to(waypoint.position);
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                waypoints.erase(waypoints.begin() + i);
                i--; // Adjust index after deletion
                g_config_system.save_waypoints_files();
            }

            ImGui::PopID();
        }
    }

    ImGui::Separator();

    // Clear all button
    if (!waypoints.empty() && ImGui::Button("Clear All Waypoints")) {
        waypoints.clear();
        g_config_system.save_waypoints_files();
    }

    // Restore styles
    ImGui::SetWindowFontScale(1.0f);
    if (g_panelFont) ImGui::PopFont();
    ImGui::PopStyleVar(2);
    ImGui::End();
}


static std::string get_target_process_name()
{
    HWND target = FindWindowA(nullptr, "Roblox");
    if (target == nullptr)
        target = GetForegroundWindow();
    if (target == nullptr)
        return std::string("unknown");

    DWORD pid = 0;
    GetWindowThreadProcessId(target, &pid);
    if (pid == 0)
        return std::string("unknown");

    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc)
        return std::string("unknown");

    wchar_t wbuf[1024];
    DWORD size = static_cast<DWORD>(std::size(wbuf));
    std::string result = "unknown";
    if (QueryFullProcessImageNameW(hProc, 0, wbuf, &size))
    {
        std::wstring full(wbuf);
        size_t pos = full.find_last_of(L"\\/");
        std::wstring fname = (pos == std::wstring::npos) ? full : full.substr(pos + 1);
        // Convert to UTF-8
        int needed = WideCharToMultiByte(CP_UTF8, 0, fname.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (needed > 0)
        {
            std::string utf8(needed - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, fname.c_str(), -1, utf8.data(), needed, nullptr, nullptr);
            result = utf8;
        }
    }
    CloseHandle(hProc);
    return result;
}


// Keybind list removed
static inline void render_keybind_list() {
    // intentionally empty


    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;



    static ImVec2 keybind_pos = ImVec2(5.0f, static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.0f - 10.0f);
    static bool first_time = true;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize;

    if (!overlay::visible) {
        window_flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
    }

    if (first_time || (!overlay::visible)) {
        ImGui::SetNextWindowPos(keybind_pos, ImGuiCond_Always);
        first_time = false;
    }

    std::vector<std::pair<std::string, std::string>> active_keybinds;

    if (globals::combat::aimbot && globals::combat::aimbotkeybind.enabled) {
        active_keybinds.push_back({ "Aimbot", globals::combat::aimbotkeybind.get_key_name() });
    }

    if (globals::combat::silentaim && globals::combat::silentaimkeybind.enabled) {
        active_keybinds.push_back({ "Silent Aim", globals::combat::silentaimkeybind.get_key_name() });
    }

    if (globals::misc::desync && globals::misc::desynckeybind.enabled) {
        active_keybinds.push_back({ "Desync", globals::misc::desynckeybind.get_key_name() });
    }

    if (globals::misc::speed && globals::misc::speedkeybind.enabled) {
        active_keybinds.push_back({ "Speed", globals::misc::speedkeybind.get_key_name() });
    }

    if (globals::misc::jumppower && globals::misc::jumppowerkeybind.enabled) {
        active_keybinds.push_back({ "Jump Power", globals::misc::jumppowerkeybind.get_key_name() });
    }

    if (globals::misc::flight && globals::misc::flightkeybind.enabled) {
        active_keybinds.push_back({ "Flight", globals::misc::flightkeybind.get_key_name() });
    }



    if (globals::misc::keybindsstyle == 1) {
        struct KeybindInfo {
            std::string name;
            keybind* bind;
            bool* enabled;
        };

        std::vector<KeybindInfo> all_keybinds = {
            {"Aimbot", &globals::combat::aimbotkeybind, &globals::combat::aimbot},
            {"Silent Aim", &globals::combat::silentaimkeybind, &globals::combat::silentaim},
            {"Desync", &globals::misc::desynckeybind, &globals::misc::desync},
            {"Speed", &globals::misc::speedkeybind, &globals::misc::speed},
            {"Jump Power", &globals::misc::jumppowerkeybind, &globals::misc::jumppower},
            {"Flight", &globals::misc::flightkeybind, &globals::misc::flight}
        };

        active_keybinds.clear();
        for (const auto& info : all_keybinds) {
            if (*info.enabled) {
                active_keybinds.push_back({ info.name, info.bind->get_key_name() });
            }
        }
    }

    ImVec2 title_size = ImGui::CalcTextSize("Keybinds");
    float content_width = title_size.x;

    for (const auto& bind : active_keybinds) {
        std::string full_text = bind.first + ": " + bind.second;
        ImVec2 text_size = ImGui::CalcTextSize(full_text.c_str());
        content_width = std::max(content_width, text_size.x);
    }

    float padding_x = 3.0f;
    float padding_y = 3.0f;
    float line_spacing = ImGui::GetTextLineHeight() + 2.0f;

    float total_width = content_width + (padding_x * 2) + 1.0f;
    float total_height = padding_y * 2 + title_size.y + 2;

    if (!active_keybinds.empty()) {
        total_height += active_keybinds.size() * line_spacing;
    }

    ImGui::SetNextWindowSize(ImVec2(total_width, total_height), ImGuiCond_Always);

    ImGui::Begin("Keybinds", nullptr, window_flags);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    if (overlay::visible) {
        keybind_pos = window_pos;
    }

    ImU32 bg_color = IM_COL32(15, 15, 15, 200);
    ImU32 text_color = IM_COL32(255, 255, 255, 255);
    ImU32 outline_color = IM_COL32(60, 60, 60, 255);
    ImU32 top_line_color = ImGui::GetColorU32(ImGuiCol_SliderGrab);
    ImU32 active_color = IM_COL32(105, 0, 255, 255);



    // Head Dot UI removed

    ImGui::Checkbox("Lock Indicator", &globals::visuals::lockedindicator);
    draw->AddRectFilled(
        window_pos,
        ImVec2(window_pos.x + window_size.x, window_pos.y + 2),
        top_line_color,
        0.0f
    );

    ImVec2 title_pos = ImVec2(
        window_pos.x + padding_x,
        window_pos.y + padding_y
    );

    draw->AddText(
        title_pos,
        text_color,
        "Keybinds"
    );

    if (!active_keybinds.empty()) {
        float current_y = title_pos.y + title_size.y + 4.0f;

        std::sort(active_keybinds.begin(), active_keybinds.end(),
            [](const std::pair<std::string, std::string>& a, const std::pair<std::string, std::string>& b) {
                std::string full_a = a.first + ": " + a.second;
                std::string full_b = b.first + ": " + b.second;
                return full_a.length() > full_b.length();
            });

        for (const auto& bind : active_keybinds) {
            std::string full_text = bind.first + ": " + bind.second;

            ImVec2 keybind_pos = ImVec2(window_pos.x + padding_x, current_y);

            if (globals::misc::keybindsstyle == 1) {
                bool is_active = false;
                if (bind.first == "Aimbot") is_active = globals::combat::aimbotkeybind.enabled;
                else if (bind.first == "Silent Aim") is_active = globals::combat::silentaimkeybind.enabled;
                else if (bind.first == "Desync") is_active = globals::misc::desynckeybind.enabled;
                else if (bind.first == "Speed") is_active = globals::misc::speedkeybind.enabled;
                else if (bind.first == "Jump Power") is_active = globals::misc::jumppowerkeybind.enabled;
                else if (bind.first == "Flight") is_active = globals::misc::flightkeybind.enabled;

                draw->AddText(keybind_pos, is_active ? active_color : text_color, full_text.c_str());
            }
            else {
                draw->AddText(keybind_pos, text_color, full_text.c_str());
            }

            current_y += line_spacing;
        }
    }
    ImGui::End();
}



bool Bind(keybind* keybind, const ImVec2& size_arg = ImVec2(0, 0), bool clicked = false, ImGuiButtonFlags flags = 0) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(keybind->get_name().c_str());
    char popup_name[128];
    snprintf(popup_name, sizeof(popup_name), "%s##%08X", keybind->get_name().c_str(), id);
    const ImVec2 label_size = ImGui::CalcTextSize(keybind->get_name().c_str(), NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
        style.FramePadding.y < window->DC.CurrLineTextBaseOffset)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;

    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_PressedOnRelease;

    bool hovered, held;
    bool Pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    bool value_changed = false;
    int key = keybind->key;

    auto io = ImGui::GetIO();

    std::string name = keybind->get_key_name();

    if (keybind->waiting_for_input) {
        name = "[Waiting]";
    }


    if (ImGui::GetIO().MouseClicked[0] && hovered)
    {
        if (g.ActiveId == id)
        {
            keybind->waiting_for_input = true;
        }
    }
    else if (ImGui::GetIO().MouseClicked[1] && hovered)
    {
        ImGui::OpenPopup(popup_name);
    }
    else if (ImGui::GetIO().MouseClicked[0] && !hovered)
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    if (keybind->waiting_for_input)
    {
        if (ImGui::GetIO().MouseClicked[0] && !hovered)
        {
            keybind->key = VK_LBUTTON;
            ImGui::ClearActiveID();
            keybind->waiting_for_input = false;
        }
        else
        {
            if (keybind->set_key())
            {
                ImGui::ClearActiveID();
                keybind->waiting_for_input = false;
            }
        }
    }

    ImVec4 bgColor = ImVec4(254.0f / 255.0f, 208.0f / 255.0f, 2.0f / 255.0f, 1.0f);
    ImVec4 borderColor = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    ImVec4 textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    // Center text, but ensure it fits within the button bounds
    ImVec2 text_size = ImGui::CalcTextSize(name.c_str());
    ImVec2 text_pos = bb.Min + ImVec2((size_arg.x > 0 ? size_arg.x : size.x) / 2 - text_size.x / 2, (size_arg.y > 0 ? size_arg.y : size.y) / 2 - text_size.y / 2);
    // Clamp text position to ensure it doesn't overflow
    text_pos.x = ImMax(bb.Min.x + style.FramePadding.x, ImMin(text_pos.x, bb.Max.x - text_size.x - style.FramePadding.x));
    window->DrawList->AddText(text_pos, ImGui::GetColorU32(textColor), name.c_str());

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;

    if (ImGui::BeginPopup(popup_name))
    {
        {


            if (ImGui::Selectable("Hold", keybind->type == keybind::HOLD)) keybind->type = keybind::HOLD;
            if (ImGui::Selectable("Toggle", keybind->type == keybind::TOGGLE)) keybind->type = keybind::TOGGLE;
            if (ImGui::Selectable("Always", keybind->type == keybind::ALWAYS)) keybind->type = keybind::ALWAYS;

        }
        ImGui::EndPopup();

    }

    return Pressed;
}

void draw_shadowed_text(const char* label) {
    ImGuiContext& g = *GImGui;

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    const ImGuiStyle& style = g.Style;
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    float HeaderHeight = ImGui::GetFontSize() + style.WindowPadding.y * 2 + style.ChildBorderSize * 2;
    pos.y = pos.y - 4;

    pDrawList->AddText(pos + style.WindowPadding + ImVec2(0, style.ChildBorderSize * 2) + ImVec2(1, 1), ImGui::GetColorU32(ImGuiCol_TitleBg), label);
    pDrawList->AddText(pos + style.WindowPadding + ImVec2(0, style.ChildBorderSize * 2) + ImVec2(-1, -1), ImGui::GetColorU32(ImGuiCol_TitleBg), label);
    pDrawList->AddText(pos + style.WindowPadding + ImVec2(0, style.ChildBorderSize * 2) + ImVec2(1, -1), ImGui::GetColorU32(ImGuiCol_TitleBg), label);
    pDrawList->AddText(pos + style.WindowPadding + ImVec2(0, style.ChildBorderSize * 2) + ImVec2(-1, 1), ImGui::GetColorU32(ImGuiCol_TitleBg), label);
    pDrawList->AddText(pos + style.WindowPadding + ImVec2(0, style.ChildBorderSize * 2), ImGui::GetColorU32(ImGuiCol_Text), label);

    ImGui::SetCursorPosY(HeaderHeight - style.WindowPadding.y + 2);
}

void overlay::load_interface()
{

    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowExW(WS_EX_TOPMOST, wc.lpszClassName, L"zoinbase", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"base";
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW;

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);



    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    SetBlackBlueTheme();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;



    ImGuiStyle& style = ImGui::GetStyle();
    // Rounded corners
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;               // Rounded inputs/sliders/buttons
    style.GrabMinSize = 3.0f;  // Even smaller slider grab handle
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(4.0f, 2.0f);
    style.ItemInnerSpacing = ImVec2(2.0f, 1.0f);  // Reduced inner spacing
    style.FramePadding = ImVec2(2.0f, 0.1f);  // Minimal padding for very compact boxes/sliders

    // Reuse the same purple/dark theme we set up in SetBlackBlueTheme so everything is consistent
    ImVec4* colors = ImGui::GetStyle().Colors;

    ImVec4 col_bg_dark = ImVec4(0.15f, 0.05f, 0.15f, 1.00f);  // Dark purple background
    ImVec4 col_bg_panel = ImVec4(0.239, 0.075, 0.380, 1.0);  // Dark purple panel
    ImVec4 col_text = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    ImVec4 col_text_disabled = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    ImVec4 col_blue = ImVec4(0.70f, 0.40f, 1.00f, 1.00f);
    ImVec4 col_blue_hover = ImVec4(0.80f, 0.50f, 1.00f, 1.00f);
    ImVec4 col_blue_active = ImVec4(0.60f, 0.30f, 1.00f, 1.00f);

    colors[ImGuiCol_Text] = col_text;
    colors[ImGuiCol_TextDisabled] = col_text_disabled;

    colors[ImGuiCol_WindowBg] = col_bg_dark;
    colors[ImGuiCol_ChildBg] = col_bg_panel;
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

    colors[ImGuiCol_Border] = ImVec4(0.60f, 0.20f, 0.60f, 1.00f); // purple border
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);  // No hover color
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);  // No active color

    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);  // No hover color
    colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);  // No active color

    colors[ImGuiCol_SliderGrab] = col_blue;
    colors[ImGuiCol_SliderGrabActive] = col_blue;  // No active color change

    colors[ImGuiCol_Header] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);  // No hover color
    colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);  // No active color

    colors[ImGuiCol_Separator] = ImVec4(0.60f, 0.20f, 0.60f, 0.85f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.20f, 0.60f, 0.85f);  // No hover color
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.60f, 0.20f, 0.60f, 0.85f);  // No active color

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.70f, 0.40f, 1.00f, 0.60f);  // Purple resize grip, more visible
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.70f, 0.40f, 1.00f, 0.80f);  // Brighter on hover
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.40f, 1.00f, 1.00f);  // Fully visible when active

    colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);  // No hover color
    colors[ImGuiCol_TabSelected] = col_blue;
    colors[ImGuiCol_TabSelectedOverline] = col_blue_active;
    colors[ImGuiCol_TabDimmed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = col_blue;
    colors[ImGuiCol_TabDimmedSelectedOverline] = col_blue_active;

    colors[ImGuiCol_PlotLines] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = col_blue;

    colors[ImGuiCol_PlotHistogram] = col_blue;
    colors[ImGuiCol_PlotHistogramHovered] = col_blue_active;

    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);

    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

    colors[ImGuiCol_TextLink] = col_blue;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(col_blue.x, col_blue.y, col_blue.z, 0.35f);

    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.90f, 0.40f, 0.90f);

    colors[ImGuiCol_NavHighlight] = ImVec4(col_blue.x, col_blue.y, col_blue.z, 0.40f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.60f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.90f);

    colors[ImGuiCol_WindowShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);






    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    // Load default font
    {
        ImFontConfig cfg; cfg.OversampleH = 1; cfg.OversampleV = 1; cfg.PixelSnapH = true;
        // Change the path below to your font file, and adjust the size (13.0f) as needed
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 12.0f, &cfg, io.Fonts->GetGlyphRangesDefault());

        // Smooth TTF font for panel (system Segoe UI if available)
        ImFontConfig smoothCfg; smoothCfg.OversampleH = 3; smoothCfg.OversampleV = 3; smoothCfg.PixelSnapH = false;
        const char* segoePath = "C:\\Windows\\Fonts\\segoeui.ttf";
        try {
            if (std::filesystem::exists(segoePath)) {
                g_panelFont = io.Fonts->AddFontFromFileTTF(segoePath, 12.0f, &smoothCfg, io.Fonts->GetGlyphRangesDefault());
            }
        }
        catch (...) { /* ignore */ }

        io.Fonts->Build();
    }

    // Dynamic font loader: scan resources/fonts and build a list for UI selection
    static std::vector<std::string> g_fontFiles;
    static int g_selectedFont = 0; // 0 = default, 1..N map to g_fontFiles entries
    static bool g_pendingFontReload = false;
    g_fontFiles.clear();
    try {
        std::string fontDir = "resources/fonts";
        if (std::filesystem::exists(fontDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(fontDir)) {
                if (!entry.is_regular_file()) continue;
                auto ext = entry.path().extension().string();
                for (auto& c : ext) c = (char)tolower((unsigned char)c);
                if (ext == ".ttf" || ext == ".otf") {
                    g_fontFiles.push_back(entry.path().string());
                }
            }
        }
    }
    catch (...) {}

    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 0.f);

    // (removed ESP font override)

    bool done = false;

    initialize_avatar_system();

    while (done == false)
    {
        if (!globals::firstreceived)return;


        auto avatar_mgr = overlay::get_avatar_manager();
        for (roblox::player entity : globals::instances::cachedplayers) {

            if (avatar_mgr) {
                if (!entity.pictureDownloaded) {
                    // Team check - skip teammates if teamcheck is enabled
                    if (globals::is_teammate(entity)) {
                        continue; // Skip teammate
                    }

                    // Performance optimization: Limit avatar requests per frame
                    static int avatar_requests_this_frame = 0;
                    static auto last_avatar_reset = std::chrono::steady_clock::now();
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_avatar_reset);

                    if (elapsed.count() >= 100) { // Reset counter every 100ms
                        avatar_requests_this_frame = 0;
                        last_avatar_reset = now;
                    }

                    if (avatar_requests_this_frame < 3) { // Max 3 avatar requests per 100ms
                        avatar_mgr->requestAvatar(entity.userid.address);
                        avatar_requests_this_frame++;
                    }
                }
                else {
                    continue;
                }

            }
            else {
                break;
            }
        }


        static HWND robloxWindow = FindWindowA(0, "Roblox");
        robloxWindow = FindWindowA(0, "Roblox");

        // Periodic Roblox process check (every 5 seconds)
        static auto lastRobloxCheck = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastRobloxCheck).count() >= 5) {
            lastRobloxCheck = currentTime;

            // Check if RobloxPlayerBeta.exe is actually running
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            bool robloxRunning = false;

            if (snapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe32;
                pe32.dwSize = sizeof(PROCESSENTRY32);

                if (Process32First(snapshot, &pe32)) {
                    do {
                        if (strcmp(pe32.szExeFile, "RobloxPlayerBeta.exe") == 0) {
                            robloxRunning = true;
                            break;
                        }
                    } while (Process32Next(snapshot, &pe32));
                }
                CloseHandle(snapshot);
            }

            if (!robloxRunning) {
                // Roblox is not running, close the application
                std::cout << "[OVERLAY] Roblox not detected, closing application..." << std::endl;
                Sleep(1000); // Give user time to see the message
                exit(0);
            }
        }

        update_avatars();

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                done = true;
                break;
            }
        }

        if (done == true)
        {
            break;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        movewindow(hwnd);







        // Disable legacy targeting flags so no other checks run
        for (size_t i = 0; i < globals::combat::flags->size(); ++i) {
            (*globals::combat::flags)[i] = 0;
        }





        if (FindWindowA(0, "Roblox") && (GetForegroundWindow() == FindWindowA(0, "Roblox") || GetForegroundWindow() == hwnd)) {
            globals::focused = true;
        }
        else {
            globals::focused = false;
        }
        if (FindWindowA(0, "Roblox") && (GetForegroundWindow() == FindWindowA(0, "Roblox") || GetForegroundWindow() == hwnd))
        {


            static bool firsssssssssssssssss = true;
            if (globals::focused && firsssssssssssssssss) {
                overlay::visible = true;
                firsssssssssssssssss = false;
            }

            // Poll menu toggle keybind instead of hardcoded keys
            {
                // Poll default or custom menu key
                int pollKey = g_menu_custom_bind_enabled ? g_menu_toggle_keybind.key : g_menu_default_key;
                if (GetAsyncKeyState(pollKey) & 1) {
                    overlay::visible = !overlay::visible;
                }
            }

            static float ui_alpha = 0.0f;
            const float fade_speed_per_sec = 8.0f; // higher = faster fade
            float target_alpha = overlay::visible ? 1.0f : 0.0f;
            float dt_fade = ImGui::GetIO().DeltaTime;
            if (ui_alpha < target_alpha) ui_alpha = ImMin(target_alpha, ui_alpha + dt_fade * fade_speed_per_sec);
            else if (ui_alpha > target_alpha) ui_alpha = ImMax(target_alpha, ui_alpha - dt_fade * fade_speed_per_sec);

            if (overlay::visible || ui_alpha > 0.01f)
            {


                static ImVec2 current_dimensions;
                // Player list removed
                current_dimensions = ImVec2(globals::instances::visualengine.GetDimensins().x, globals::instances::visualengine.GetDimensins().y);

                ImGui::GetBackgroundDrawList()->AddRectFilled(
                    ImVec2(0, 0),
                    current_dimensions,
                    ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.12f, 0.89f * ui_alpha)),
                    0
                );

                // Snow particles background - Performance optimized
                {
                    struct Snowflake { ImVec2 pos; float speed; float drift; float size; float phase; };
                    static std::vector<Snowflake> flakes;
                    static ImVec2 last_screen = ImVec2(0, 0);
                    static bool seeded = false;
                    static auto last_snow_update = std::chrono::steady_clock::now();

                    if (!seeded) { srand((unsigned)time(nullptr)); seeded = true; }

                    ImVec2 screen = ImGui::GetIO().DisplaySize;
                    const int target_count = 70; // Reduced from 140

                    if (flakes.empty() || screen.x != last_screen.x || screen.y != last_screen.y) {
                        flakes.clear(); flakes.reserve(target_count);
                        last_screen = screen;
                        for (int i = 0; i < target_count; ++i) {
                            Snowflake f;
                            f.pos = ImVec2((float)(rand() % (int)std::max(1.0f, screen.x)), (float)(rand() % (int)std::max(1.0f, screen.y)));
                            f.speed = 30.0f + (float)(rand() % 50); // Reduced speed range
                            f.drift = 10.0f + (float)(rand() % 20); // Reduced drift
                            f.size = 1.0f + (float)(rand() % 150) / 100.0f; // Smaller size range
                            f.phase = (float)(rand() % 628) / 100.0f;
                            flakes.push_back(f);
                        }
                    }

                    // Performance optimization: Update snow less frequently
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_snow_update);
                    if (elapsed.count() < 16) { // ~60 FPS instead of every frame
                        // Skip update but still render
                    }
                    else {
                        float dt = ImGui::GetIO().DeltaTime;
                        for (auto& f : flakes) {
                            f.phase += dt * 0.8f; // Reduced phase speed
                            f.pos.y += f.speed * dt;
                            f.pos.x += cosf(f.phase) * f.drift * dt;

                            if (f.pos.y > screen.y + 8.0f) {
                                f.pos.y = -8.0f;
                                f.pos.x = (float)(rand() % (int)std::max(1.0f, screen.x));
                                f.speed = 30.0f + (float)(rand() % 50);
                                f.drift = 10.0f + (float)(rand() % 20);
                                f.size = 1.0f + (float)(rand() % 150) / 100.0f;
                            }
                            if (f.pos.x < -8.0f) f.pos.x = screen.x + 8.0f;
                            if (f.pos.x > screen.x + 8.0f) f.pos.x = -8.0f;
                        }
                        last_snow_update = now;
                    }

                    ImDrawList* drawbglist = ImGui::GetBackgroundDrawList();
                    for (auto& f : flakes) {
                        drawbglist->AddCircleFilled(f.pos, f.size, IM_COL32(255, 255, 255, 150)); // Reduced alpha
                    }
                }

            }
            if (globals::combat::drawfov) {
                POINT cursor_pos;
                GetCursorPos(&cursor_pos);
                ScreenToClient(robloxWindow, &cursor_pos);
                ImVec2 mousepos = ImVec2(static_cast<float>(static_cast<int>(cursor_pos.x)), static_cast<float>(static_cast<int>(cursor_pos.y)));
                ImDrawList* drawbglist = ImGui::GetBackgroundDrawList();
                drawbglist->AddCircle(mousepos, globals::combat::fovsize - 1, IM_COL32(0, 0, 0, 255));
                drawbglist->AddCircle(mousepos, globals::combat::fovsize, ImGui::ColorConvertFloat4ToU32(ImVec4(globals::combat::fovcolor[0], globals::combat::fovcolor[1], globals::combat::fovcolor[2], globals::combat::fovcolor[3])));
                drawbglist->AddCircle(mousepos, globals::combat::fovsize + 1, IM_COL32(0, 0, 0, 255));
            }
            if (globals::combat::drawsfov) {
                POINT cursor_pos;
                GetCursorPos(&cursor_pos);
                ScreenToClient(robloxWindow, &cursor_pos);
                ImVec2 mousepos = ImVec2(static_cast<float>(static_cast<int>(cursor_pos.x)), static_cast<float>(static_cast<int>(cursor_pos.y)));
                ImDrawList* drawbglist = ImGui::GetBackgroundDrawList();
                drawbglist->AddCircle(mousepos, globals::combat::sfovsize - 1, IM_COL32(0, 0, 0, 255));
                drawbglist->AddCircle(mousepos, globals::combat::sfovsize, ImGui::ColorConvertFloat4ToU32(ImVec4(globals::combat::sfovcolor[0], globals::combat::sfovcolor[1], globals::combat::sfovcolor[2], globals::combat::sfovcolor[3])));
                drawbglist->AddCircle(mousepos, globals::combat::sfovsize + 1, IM_COL32(0, 0, 0, 255));
            }

            // Waypoints removed
             // removed hardcoded menu toggle keys; handled by g_menu_toggle_keybind above
            if (overlay::visible || ui_alpha > 0.01f)
            {

                style.WindowShadowSize = 0.0f;
                style.Colors[ImGuiCol_WindowShadow] = style.Colors[ImGuiCol_SliderGrab];
                // Tweak labeled separator appearance for clearer section separation
                style.SeparatorTextBorderSize = 1.0f;
                style.SeparatorTextPadding = ImVec2(10.0f, 2.0f);
                style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
                ImGui::PopStyleColor(1);

                // Make the main menu smaller for a more compact layout
                ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_Once);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ui_alpha);

                // Waypoint panel removed

                ImGuiWindowFlags layuh_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
                if (!overlay::visible) layuh_flags |= ImGuiWindowFlags_NoInputs;
                if (ImGui::Begin("zoinbase", nullptr, layuh_flags))
                {
                    ImDrawList* dl_root = ImGui::GetWindowDrawList();
                    ImVec2 wpos = ImGui::GetWindowPos();
                    ImVec2 wsize = ImGui::GetWindowSize();

                    // Top bar with "zoinbase.larp" title
                    float header_h = 40.0f;
                    ImU32 header_bg = ImGui::GetColorU32(ImVec4(0.10f, 0.10f, 0.10f, 1.00f));  // Dark grey header
                    dl_root->AddRectFilled(wpos, ImVec2(wpos.x + wsize.x, wpos.y + header_h), header_bg);

                    ImFont* fnt = ImGui::GetFont();
                    float fs = ImGui::GetFontSize();

                    // Draw "zoinbase.larp" with ".larp" in purple
                    const char* t1 = "Luftwaffle";
                    const char* t2 = " | Dev";
                    ImVec2 s1 = fnt->CalcTextSizeA(fs, FLT_MAX, 0.0f, t1);
                    ImVec2 s2 = fnt->CalcTextSizeA(fs, FLT_MAX, 0.0f, t2);
                    float tx = wpos.x + 20.0f; // 20px margin from left
                    float ty = wpos.y + (header_h - fs) * 0.5f;
                    // Title "Luftwaffle | Dev" with purple accent on " | Dev"
                    dl_root->AddText(ImVec2(tx, ty), IM_COL32(255, 255, 255, 255), t1);// 143, 112, 171, 0.8
                    dl_root->AddText(ImVec2(tx + s1.x, ty), IM_COL32(143, 112, 171, 255), t2); // IGNORE - Blue color 0.92f, 0.92f, 0.92f, 1.00f

                    // Move content below header
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + header_h);

                    // Top navigation bar with horizontal tabs
                    static int selected_tab = 0;
                    const char* tabNames[] = { "Combat", "Silent", "Misc", "Checks", "Visuals", "Movement", "Menu", "Config" };
                    const int tabCount = sizeof(tabNames) / sizeof(tabNames[0]);

                    // Navigation bar container
                    ImGui::BeginChild("NavBar", ImVec2(0, 45.0f), false, ImGuiWindowFlags_NoScrollbar);
                    ImDrawList* nav_draw = ImGui::GetWindowDrawList();
                    ImVec2 nav_pos = ImGui::GetWindowPos();
                    ImVec2 nav_size = ImGui::GetWindowSize();
                    
                    // Background for nav bar - transparent dark purple
                    ImU32 nav_bg = ImGui::GetColorU32(ImVec4(0.239, 0.075, 0.380, 1.0)); // 0.60f, 0.20f, 0.60f, 1.00f - PRIMARY ACCENT OF UI
                    nav_draw->AddRectFilled(nav_pos, ImVec2(nav_pos.x + nav_size.x, nav_pos.y + nav_size.y), nav_bg);
                    
                    // Calculate button width
                    float button_width = (nav_size.x - 40.0f) / tabCount; // 40px total padding (20px each side)
                    float button_height = 35.0f;
                    float button_spacing = 5.0f;
                    float start_x = nav_pos.x + 20.0f;
                    
                    for (int i = 0; i < tabCount; ++i) {
                        ImGui::PushID(i);
                        
                        float btn_x = start_x + (button_width + button_spacing) * i;
                        float btn_y = nav_pos.y + (nav_size.y - button_height) * 0.5f;
                        ImVec2 btn_pos = ImVec2(btn_x, btn_y);
                        ImVec2 btn_size = ImVec2(button_width, button_height);
                        
                        bool is_selected = (selected_tab == i);
                        bool is_hovered = ImGui::IsMouseHoveringRect(btn_pos, ImVec2(btn_pos.x + btn_size.x, btn_pos.y + btn_size.y));
                        
                        // Button background
                        ImU32 btn_bg;
                        if (is_selected) {
                            btn_bg = ImGui::GetColorU32(ImVec4(0.294, 0.184, 0.388, 1.0));
                        } else if (is_hovered) {
                            btn_bg = ImGui::GetColorU32(ImVec4(0.365, 0.212, 0.502, 1.0));
                        } else {
                            btn_bg = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                        }
                        
                        nav_draw->AddRectFilled(btn_pos, ImVec2(btn_pos.x + btn_size.x, btn_pos.y + btn_size.y), btn_bg, 4.0f);
                        
                        // Text color
                        ImU32 text_color;
                        if (is_selected) { // 0.533, 0.0, 1.0, 1.0
                            text_color = IM_COL32(147, 84, 235, 255); // Blue for selected 147, 84, 235
                        } else {
                            text_color = IM_COL32(255, 255, 255, 255); // Gray for unselected
                        }
                        
                        // Center text in button
                        ImFont* fnt = ImGui::GetFont();
                        float fs = ImGui::GetFontSize();
                        ImVec2 text_size = fnt->CalcTextSizeA(fs, FLT_MAX, 0.0f, tabNames[i]);
                        ImVec2 text_pos = ImVec2(btn_pos.x + (btn_size.x - text_size.x) * 0.5f, btn_pos.y + (btn_size.y - text_size.y) * 0.5f);
                        nav_draw->AddText(text_pos, text_color, tabNames[i]);
                        
                        // Handle click
                        if (ImGui::IsMouseClicked(0) && is_hovered) {
                            selected_tab = i;
                        }
                        
                        ImGui::PopID();
                    }
                    
                    ImGui::EndChild();
                    
                    // Add spacing below nav bar
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Main content area - full width now
                    ImGui::BeginChild("ContentArea", ImVec2(0, 0), false);

                    // Content area
                    if (selected_tab == 0) // Combat tab
                    {
                        // Combat section - two columns (j1e style)
                        ImGui::Columns(2, nullptr, false);
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("Child1", ImVec2(0, 0), true);
                        ImGui::TextDisabled("Aimbot Settings");
                        ImGui::Spacing();
                        ImGui::Checkbox("Enable", &globals::combat::aimbot);
                        ImGui::SameLine();
                        // Calculate width based on available space, but ensure minimum width for keybind button
                        float keybind_width = ImMax(70.0f, ImGui::GetContentRegionAvail().x * 0.3f);
                        Bind(&globals::combat::aimbotkeybind, ImVec2(keybind_width, 20.0f));
                        ImGui::Checkbox("Sticky Aim", &globals::combat::stickyaim);
                        
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::Combo("Type", &globals::combat::aimbottype, "Camera\0Mouse\0Memory\0");

                        // Target method selection
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::Combo("Target Method", &globals::combat::target_method, "Closest To Mouse\0Closest To Camera\0");

                        if (globals::combat::aimbottype == 0) {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::Combo("Camlock Body Part", &globals::combat::aimpart, "Head\0Upper Torso\0Lower Torso\0HumanoidRootPart\0Left Hand\0Right Hand\0Left Foot\0Right Foot\0Closest Part\0Random Part\0");
                        }
                        else if (globals::combat::aimbottype == 1) {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::Combo("Mouselock Body Part", &globals::combat::aimpart, "Head\0Upper Torso\0Lower Torso\0HumanoidRootPart\0Left Hand\0Right Hand\0Left Foot\0Right Foot\0Closest Part\0Random Part\0");
                        }
                        else {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::Combo("Memory Body Part", &globals::combat::aimpart, "Head\0Upper Torso\0Lower Torso\0HumanoidRootPart\0Left Hand\0Right Hand\0Left Foot\0Right Foot\0Closest Part\0Random Part\0");
                        }

                        // Airpart feature
                        ImGui::Checkbox("Airpart", &globals::combat::airpart_enabled);
                        if (globals::combat::airpart_enabled) {
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::Combo("Airpart Body Part", &globals::combat::airpart, "Head\0Upper Torso\0Lower Torso\0HumanoidRootPart\0Left Hand\0Right Hand\0Left Foot\0Right Foot\0Closest Part\0Random Part\0");
                        }

                        ImGui::Checkbox("Show FOV", &globals::combat::drawfov);
                        ImGui::SameLine();
                        BeginVisualDisabled(!globals::combat::drawfov);
                        ImGui::ColorEdit4("FOV Color", globals::combat::fovcolor, ImGuiColorEditFlags_NoInputs);
                        EndVisualDisabled();
                        ImGui::Checkbox("Use FOV", &globals::combat::usefov);
                        BeginVisualDisabled(!globals::combat::usefov);
                        { // Slider
                            ImGui::Text("Radius");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::combat::fovsize).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::combat::fovsize).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##Radius", &globals::combat::fovsize, 10.f, 1000.f);
                        }
                        EndVisualDisabled();

                        // No Sleep Aimbot option for Memory and Camera types only
                        if (globals::combat::aimbottype == 0 || globals::combat::aimbottype == 2) {
                            ImGui::Spacing();
                            ImGui::Checkbox("No Sleep Aimbot", &globals::combat::nosleep_aimbot);
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Removes sleep delays for faster aimbot performance (Memory and Camera types only)");
                            }
                        }

                        ImGui::EndChild();

                        ImGui::NextColumn();
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("Child2", ImVec2(0, 0), true);
                        ImGui::TextDisabled("Advanced Settings");
                        ImGui::Spacing();

                        if (globals::combat::aimbottype == 0 || globals::combat::aimbottype == 2) {
                            ImGui::Checkbox("Enable##prediction", &globals::combat::predictions);
                            ImGui::SameLine();
                            static int camlockPredPreset = 0; // 0: Custom, 1: Low, 2: Medium, 3: High
                            BeginVisualDisabled(!globals::combat::predictions);
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            if (ImGui::Combo("##PresetCamlockPred", &camlockPredPreset, "Custom\0Low\0Medium\0High\0")) {
                                switch (camlockPredPreset) {
                                case 1: globals::combat::predictionsx = 6.f; globals::combat::predictionsy = 6.f; break;
                                case 2: globals::combat::predictionsx = 10.f; globals::combat::predictionsy = 10.f; break;
                                case 3: globals::combat::predictionsx = 15.f; globals::combat::predictionsy = 15.f; break;
                                default: break; // Custom leaves sliders as-is
                                }
                            }
                            { // Slider
                                ImGui::Text("Predict X");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::combat::predictionsx).c_str()).x + 7);
                                ImGui::Text(std::to_string((int)globals::combat::predictionsx).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##PredictX", &globals::combat::predictionsx, 0.1f, 50.f);
                            }
                            { // Slider
                                ImGui::Text("Predict Y");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::combat::predictionsy).c_str()).x + 7);
                                ImGui::Text(std::to_string((int)globals::combat::predictionsy).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##PredictY", &globals::combat::predictionsy, 0.1f, 50.f);
                            }
                            EndVisualDisabled();
                        }

                        ImGui::Checkbox("Enable##deadzone", &globals::combat::deadzone);
                        BeginVisualDisabled(!globals::combat::deadzone);
                        { // Slider
                            ImGui::Text("Deadzone X");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::combat::deadzonex).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::combat::deadzonex).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##DeadzoneX", &globals::combat::deadzonex, 0.0f, 100.0f, "%.1f");
                        }
                        { // Slider
                            ImGui::Text("Deadzone Y");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::combat::deadzoney).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::combat::deadzoney).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##DeadzoneY", &globals::combat::deadzoney, 0.0f, 100.0f, "%.1f");
                        }
                        EndVisualDisabled();

                        ImGui::Checkbox("Enable##smoothing", &globals::combat::smoothing);
                        BeginVisualDisabled(!globals::combat::smoothing);
                        { // Slider
                            ImGui::Text("Smoothing X");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::combat::smoothingx).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::combat::smoothingx).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##SmoothingX", &globals::combat::smoothingx, 1.f, 50.f);
                        }
                        { // Slider
                            ImGui::Text("Smoothing Y");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::combat::smoothingy).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::combat::smoothingy).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##SmoothingY", &globals::combat::smoothingy, 1.f, 50.f);
                        }
                        const char* smoothItems =
                            "None\0Linear\0EaseInQuad\0EaseOutQuad\0EaseInOutQuad\0"
                            "EaseInCubic\0EaseOutCubic\0EaseInOutCubic\0"
                            "EaseInSine\0EaseOutSine\0EaseInOutSine\0";
                        ImGui::Text("Style");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::Combo("##StyleSmoothing", &globals::combat::smoothingstyle, smoothItems);
                        EndVisualDisabled();

                        // Camlock Shake controls moved above sensitivity
                        if (globals::combat::aimbottype == 0 || globals::combat::aimbottype == 2) {
                            ImGui::Checkbox("Enable##shake", &globals::combat::camlock_shake);
                            BeginVisualDisabled(!globals::combat::camlock_shake);
                            { // X Shake Slider
                                ImGui::Text("Shake X Intensity");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::combat::camlock_shake_x).substr(0, 4).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::combat::camlock_shake_x).substr(0, 4).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##CamlockShakeX", &globals::combat::camlock_shake_x, 0.1f, 10.0f, "%.2f");
                            }
                            { // Y Shake Slider
                                ImGui::Text("Shake Y Intensity");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::combat::camlock_shake_y).substr(0, 4).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::combat::camlock_shake_y).substr(0, 4).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##CamlockShakeY", &globals::combat::camlock_shake_y, 0.1f, 10.0f, "%.2f");
                            }
                            EndVisualDisabled();
                        }

                        ImGui::TextDisabled("Sensitivity");
                        ImGui::Spacing();
                        ImGui::Checkbox("Enable Sensitivity", &globals::combat::sensitivity_enabled);
                        BeginVisualDisabled(!globals::combat::sensitivity_enabled);
                        if (globals::combat::aimbottype == 0 || globals::combat::aimbottype == 2) {
                            { // Slider
                                ImGui::Text("Camlock Sensitivity");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::combat::cam_sensitivity).substr(0, 4).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::combat::cam_sensitivity).substr(0, 4).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##CamlockSensitivity", &globals::combat::cam_sensitivity, 0.1f, 5.0f, "%.2f");
                            }
                        }
                        else if (globals::combat::aimbottype == 1) {
                            { // Slider
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 150);
                                ImGui::SliderFloat("##MouselockSensitivity", &globals::combat::mouse_sensitivity, 0.05f, 5.0f, "%.2f");
                                ImGui::SameLine();
                                ImGui::Text("%.2f", globals::combat::mouse_sensitivity);
                                ImGui::SameLine();
                                ImGui::Text("Mouselock Sensitivity");
                            }
                        }
                        else {
                            { // Slider
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 170);
                                ImGui::SliderFloat("##MemoryAimbotSensitivity", &globals::combat::cam_sensitivity, 0.1f, 5.0f, "%.2f");
                                ImGui::SameLine();
                                ImGui::Text("%.2f", globals::combat::cam_sensitivity);
                                ImGui::SameLine();
                                ImGui::Text("Memory Aimbot Sensitivity");
                            }
                        }
                        EndVisualDisabled();

                        ImGui::EndChild();
                        ImGui::Columns(1);

                    }
                    else if (selected_tab == 1) // Silent tab
                    {
                        // Silent Aim section - two columns (j1e style)
                        ImGui::Columns(2, nullptr, false);
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("SilentAimLeft", ImVec2(0, 0), true);
                        ImGui::TextDisabled("Silent Aim Settings [NOT FINISHED]");
                        ImGui::Spacing();
                        ImGui::Checkbox("Enable##silent", &globals::combat::silentaim);
                        ImGui::SameLine();
                        float keybind_width_silent = ImMax(70.0f, ImGui::GetContentRegionAvail().x * 0.3f);
                        Bind(&globals::combat::silentaimkeybind, ImVec2(keybind_width_silent, 20.0f));
                        ImGui::Checkbox("Spoof Mouse##silent", &globals::combat::spoofmouse);
                        ImGui::Checkbox("Sticky Aim##silent", &globals::combat::stickyaimsilent);
                        //ImGui::Checkbox("Connect to Aimbot##silent", &globals::combat::connect_to_aimbot);
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80);
                        ImGui::Combo("##BodyPartSilent", &globals::combat::silentaimpart, "Head\0Upper Torso\0Lower Torso\0HumanoidRootPart\0Left Hand\0Right Hand\0Left Foot\0Right Foot\0Closest Part\0Random Part\0");
                        ImGui::SameLine();
                        ImGui::Text("Body Part");
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 120);
                        ImGui::Combo("##ClosestPartModeSilent", &globals::combat::closestpartsilent, "Off\0Closest Part\0Closest Point\0");
                        ImGui::SameLine();
                        ImGui::Text("Closest Part Mode");
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100);
                        ImGui::Combo("##TargetMethodSilent", &globals::combat::target_method, "Closest To Mouse\0Closest To Camera\0");
                        ImGui::SameLine();
                        ImGui::Text("Target Method");

                        ImGui::Checkbox("Show FOV##silent", &globals::combat::drawsfov);
                        ImGui::SameLine();
                        BeginVisualDisabled(!globals::combat::drawsfov);
                        ImGui::ColorEdit4("##FOVColorSilent", globals::combat::sfovcolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::SameLine();
                        ImGui::Text("FOV Color");
                        EndVisualDisabled();
                        ImGui::Checkbox("Use FOV##silent", &globals::combat::usesfov);
                        BeginVisualDisabled(!globals::combat::usesfov);
                        { // Slider
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80);
                            ImGui::SliderFloat("##FOVSizeSilent", &globals::combat::sfovsize, 10.f, 1000.f);
                            ImGui::SameLine();
                            ImGui::Text("%.0f", globals::combat::sfovsize);
                            ImGui::SameLine();
                            ImGui::Text("FOV Size");
                        }
                        EndVisualDisabled();

                        ImGui::EndChild();

                        ImGui::NextColumn();
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("SilentAimRight", ImVec2(0, 0), true);
                        ImGui::TextDisabled("Advanced Settings");
                        ImGui::Spacing();

                        ImGui::Checkbox("Predictions##silent", &globals::combat::silentpredictions);
                        BeginVisualDisabled(!globals::combat::silentpredictions);
                        { // Slider
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100);
                            ImGui::SliderFloat("##PredictXSilent", &globals::combat::silentpredictionsx, 0.1f, 50.f);
                            ImGui::SameLine();
                            ImGui::Text("%.0f", globals::combat::silentpredictionsx);
                            ImGui::SameLine();
                            ImGui::Text("Predict X");
                        }
                        { // Slider
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100);
                            ImGui::SliderFloat("##PredictYSilent", &globals::combat::silentpredictionsy, 0.1f, 50.f);
                            ImGui::SameLine();
                            ImGui::Text("%.0f", globals::combat::silentpredictionsy);
                            ImGui::SameLine();
                            ImGui::Text("Predict Y");
                        }
                        EndVisualDisabled();

                        ImGui::TextDisabled("Hit Chance");
                        ImGui::Spacing();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100);
                        ImGui::SliderInt("##HitChanceSilent", &globals::combat::hitchance, 1, 100);
                        ImGui::SameLine();
                        ImGui::Text("%d%%", globals::combat::hitchance);
                        ImGui::SameLine();
                        ImGui::Text("Hit Chance");

                        ImGui::EndChild();
                        ImGui::Columns(1);
                    }
                    else if (selected_tab == 2) // Misc tab
                    {
                        // Misc section
                        ImGui::Columns(2, nullptr, false);
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("MiscLeft", ImVec2(0, 0), true);

                        // Desync
                        ImGui::Checkbox("Desync", &globals::misc::desync);
                        ImGui::SameLine();
                        float keybind_width_desync = ImMax(70.0f, ImGui::GetContentRegionAvail().x * 0.3f);
                        Bind(&globals::misc::desynckeybind, ImVec2(keybind_width_desync, 20.0f));

                        // RapidFire functionality
                        ImGui::Checkbox("RapidFire V1 [Da Hood]", &globals::misc::dahood_rapidfire);
                        ImGui::Checkbox("RapidFire V2 [Arsenal]", &globals::misc::arsenal_rapidfire);
                        ImGui::SliderFloat("RapidFire Rate Value", &globals::misc::rapid_fire_value, 0, 20);

                        ImGui::Checkbox("HUD", &globals::misc::targethud);

                        // Spam TP to target
                        ImGui::Checkbox("Spam TP to Target", &globals::misc::spam_tp);

                        // Custom Models section
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::TextDisabled("Custom Models");
                        ImGui::Spacing();

                        ImGui::Checkbox("Enable Custom Models", &globals::misc::custom_model_enabled);

                        if (globals::misc::custom_model_enabled) {
                            static char custom_model_path_buffer[256] = "";
                            if (custom_model_path_buffer[0] == '\0') {
                                strcpy_s(custom_model_path_buffer, globals::misc::custom_model_folder_path.c_str());
                            }

                            ImGui::Text("Custom Models:");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80.0f);
                            if (ImGui::InputText("##CustomModelPath", custom_model_path_buffer, sizeof(custom_model_path_buffer))) {
                                globals::misc::custom_model_folder_path = std::string(custom_model_path_buffer);
                            }

                            ImGui::SameLine();
                            if (ImGui::Button("Set Path", ImVec2(70.0f, 0.0f))) {
                                std::lock_guard<std::mutex> lock(globals::misc::custom_model_mutex);
                                globals::misc::custom_model_folder_path = std::string(custom_model_path_buffer);
                            }

                            // Quick path buttons
                            ImGui::Spacing();
                            ImGui::Text("Quick Paths:");
                            if (ImGui::Button("Workspace.Bots", ImVec2(120.0f, 0.0f))) {
                                strcpy_s(custom_model_path_buffer, "Workspace.Bots");
                                std::lock_guard<std::mutex> lock(globals::misc::custom_model_mutex);
                                globals::misc::custom_model_folder_path = "Workspace.Bots";
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Workspace.Mobs", ImVec2(120.0f, 0.0f))) {
                                strcpy_s(custom_model_path_buffer, "Workspace.Mobs");
                                std::lock_guard<std::mutex> lock(globals::misc::custom_model_mutex);
                                globals::misc::custom_model_folder_path = "Workspace.Mobs";
                            }

                            // Current input display with status colors
                            ImGui::Spacing();
                            if (!globals::misc::custom_model_folder_path.empty()) {
                                try {
                                    roblox::instance test_folder = ResolveRobloxPath(globals::misc::custom_model_folder_path);
                                    if (test_folder.is_valid()) {
                                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Current input: %s", custom_model_path_buffer);
                                    }
                                    else {
                                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Current input: %s", custom_model_path_buffer);
                                    }
                                }
                                catch (...) {
                                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Current input: %s", custom_model_path_buffer);
                                }
                            }
                            else {
                                ImGui::Text("Current input: %s", custom_model_path_buffer);
                            }
                        }

                        ImGui::EndChild();

                        ImGui::NextColumn();
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("MiscRight", ImVec2(0, 0), true);

                        // Desync Color (always visible)
                        ImGui::ColorEdit4("Desync", globals::misc::desync_viz_color, ImGuiColorEditFlags_NoInputs);

                        ImGui::EndChild();
                        ImGui::Columns(1);
                    }
                    else if (selected_tab == 3) // Checks tab
                    {
                        // Checks/Filters section
                        ImGui::Columns(2, nullptr, false);
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("ChecksLeft", ImVec2(0, 0), true);
                        ImGui::Checkbox("Team Check", &globals::combat::teamcheck);
                        ImGui::Checkbox("Grabbed Check", &globals::combat::grabbedcheck);
                        ImGui::Checkbox("Unlock on death (Camlock)", &globals::combat::unlockondeath);
                        ImGui::Checkbox("Arsenal Flick Fix", &globals::combat::arsenal_flick_fix);

                        // Wallcheck
                        static bool wallcheck_prev_state = false;
                        if (ImGui::Checkbox("Wallcheck", &globals::misc::wallcheck)) {
                            if (globals::misc::wallcheck && !wallcheck_prev_state) {
                                // Enable wallcheck
                                wallcheck::initialize();
                            }
                            else if (!globals::misc::wallcheck && wallcheck_prev_state) {
                                // Disable wallcheck
                                wallcheck::shutdown();
                            }
                            wallcheck_prev_state = globals::misc::wallcheck;
                        }
                        else {
                            // Sync state if checkbox wasn't clicked but state changed externally
                            if (globals::misc::wallcheck != wallcheck_prev_state) {
                                if (globals::misc::wallcheck) {
                                    wallcheck::initialize();
                                }
                                else {
                                    wallcheck::shutdown();
                                }
                                wallcheck_prev_state = globals::misc::wallcheck;
                            }
                        }

                        ImGui::EndChild();

                        ImGui::NextColumn();
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("ChecksRight", ImVec2(0, 0), true);
                        // Add any additional filter options here
                        ImGui::Text("Additional filter options can be added here.");

                        ImGui::EndChild();
                        ImGui::Columns(1);
                    }
                    else if (selected_tab == 4) // Visuals tab
                    {
                        auto multi_select_combo = [](const char* label, std::vector<int>* flags, const std::vector<const char*>& items)
                            {
                                int selected_count = std::accumulate(flags->begin(), flags->end(), 0);
                                std::string preview;
                                if (selected_count == 0) preview = "None";
                                else if ((size_t)selected_count == items.size()) preview = "All";
                                else preview = std::to_string(selected_count) + " selected";

                                bool changed = false;
                                if (ImGui::BeginCombo(label, preview.c_str()))
                                {
                                    for (size_t i = 0; i < items.size(); ++i)
                                    {
                                        bool is_selected = (*flags)[i] != 0;
                                        if (ImGui::Selectable(items[i], is_selected, ImGuiSelectableFlags_DontClosePopups))
                                        {
                                            (*flags)[i] = is_selected ? 0 : 1;
                                            changed = true;
                                        }
                                    }
                                    ImGui::EndCombo();
                                }
                                return changed;
                            };

                        // Split Visuals into a main settings column with a Colors section at the bottom

                        ImVec2 vis_avail = ImGui::GetContentRegionAvail();
                        float left_w = vis_avail.x * 0.55f;
                        ImGui::BeginChild("VisualsSettings", ImVec2(left_w, 0), true);
                        ImGui::Checkbox("MasterSwitch", &globals::visuals::visuals);
                        // Row: main toggle + overlay checkboxes inline (colors moved to Colors section)
                        {
                            ImGui::Checkbox("Boxes", &globals::visuals::boxes);
                            // Force outline ON always
                            (*globals::visuals::box_overlay_flags)[0] = 1;
                        }
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::Combo("Box Type", &globals::visuals::boxtype, "Bounding\0Corners\0");

                        // Box Fill option
                        bool box_fill = (*globals::visuals::box_overlay_flags)[2] != 0;
                        if (ImGui::Checkbox("Fill##boxovr", &box_fill)) (*globals::visuals::box_overlay_flags)[2] = box_fill ? 1 : 0;

                        // Box Gradient option (only show when fill is enabled)
                        if (box_fill) {
                            ImGui::SameLine();
                            if (ImGui::Checkbox("Gradient##boxgrad", &globals::visuals::box_gradient)) {
                                // Gradient checkbox logic
                            }

                            // Gradient rotation: always enabled when gradient is on
                            if (globals::visuals::box_gradient) {
                                globals::visuals::box_gradient_rotation = true;
                                globals::visuals::box_gradient_rotation_speed = 0.7f;
                            }
                        }

                        ImGui::Checkbox("Name", &globals::visuals::name);
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::Combo("Name Type", &globals::visuals::nametype, "Username\0Display Name\0");

                        // Row: main toggle + overlay checkboxes inline (colors moved to Colors section)
                        {
                            ImGui::Checkbox("Health Bar", &globals::visuals::healthbar);
                            ImGui::SameLine();
                            ImGui::Checkbox("Health Text", &globals::visuals::healthtext);
                            // Force outline ON always
                            (*globals::visuals::healthbar_overlay_flags)[0] = 1;
                            ImGui::SameLine();
                            bool hb_gradient = (*globals::visuals::healthbar_overlay_flags)[1] != 0;
                            bool hb_glow = false; (*globals::visuals::healthbar_overlay_flags)[2] = 0;
                            if (ImGui::Checkbox("Gradient##healthovr", &hb_gradient)) (*globals::visuals::healthbar_overlay_flags)[1] = hb_gradient ? 1 : 0;
                        }

                        ImGui::Checkbox("Distance", &globals::visuals::distance);

                        ImGui::Checkbox("Tool Name", &globals::visuals::toolesp);

                        // Da Hood: Armor bar
                        ImGui::Checkbox("Armor Bar##armorbar", &globals::visuals::armorbar);

                        // Row: main toggle + overlay checkboxes inline (colors moved)
                        {
                            ImGui::Checkbox("Skeleton", &globals::visuals::skeletons);
                            ImGui::SameLine();
                            bool sk_outline = (*globals::visuals::skeleton_overlay_flags)[0] != 0;
                            if (ImGui::Checkbox("Outline##skelovr", &sk_outline)) (*globals::visuals::skeleton_overlay_flags)[0] = sk_outline ? 1 : 0;
                        }

                        ImGui::Checkbox("China Hat", &globals::visuals::chinahat);
                        ImGui::SameLine();
                        ImGui::Checkbox("Target", &globals::visuals::chinahat_target_only);

                        ImGui::Checkbox("Player Chams", &globals::visuals::chams_outline_s);

                        ImGui::Checkbox("Tracers##tracers", &globals::visuals::tracers);
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::Combo("Type##tracers", &globals::visuals::tracerstype, "Torso\0Normal\0Spiderweb\0");

                        ImGui::Checkbox("Trail##trail", &globals::visuals::trail);
                        if (globals::visuals::trail) {
                            ImGui::SliderFloat("Duration", &globals::visuals::trail_duration, 0.5f, 10.0f, "%.1fs");
                            ImGui::SliderFloat("Thickness", &globals::visuals::trail_thickness, 0.5f, 5.0f, "%.1f");
                            ImGui::ColorEdit4("Color##trail", globals::visuals::trail_color, ImGuiColorEditFlags_NoInputs);
                        }


                        ImGui::Checkbox("Flags", &globals::visuals::flags);
                        // Removed Flags Color/Outline/Glow options per request

                        ImGui::Checkbox("Allow LocalPlayer ESP", &globals::visuals::selfesp);

                        ImGui::Checkbox("Max Distance", &globals::visuals::maxdistance_enabled);
                        if (globals::visuals::maxdistance_enabled)
                        { // Slider
                            ImGui::Text("Max Distance Value");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::visuals::maxdistance).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::visuals::maxdistance).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##MaxDistance", &globals::visuals::maxdistance, 50.0f, 2000.0f, "%.0f");
                        }

                        // Head Dot feature removed

                        ImGui::Checkbox("Sonar##sonar", &globals::visuals::sonar);
                        if (globals::visuals::sonar) {
                            { // Slider
                                ImGui::Text("Max Range##sonar");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::visuals::sonar_range).c_str()).x + 7);
                                ImGui::Text(std::to_string((int)globals::visuals::sonar_range).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##MaxRangeSonar", &globals::visuals::sonar_range, 0.0f, 100.0f, "%.0f");
                            }
                            { // Slider
                                ImGui::Text("Circle Thickness##sonar");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::visuals::sonar_thickness).substr(0, 3).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::visuals::sonar_thickness).substr(0, 3).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##CircleThicknessSonar", &globals::visuals::sonar_thickness, 0.5f, 5.0f, "%.1f");
                            }
                        }

                        bool fog_enabled = globals::visuals::fog;
                        if (ImGui::Checkbox("Fog##fog", &fog_enabled)) {
                            globals::visuals::fog = fog_enabled;
                            if (fog_enabled) {
                                globals::visuals::fog_start = 0.0f; // Automatically set fog start to 0
                            }
                        }
                        if (globals::visuals::fog) {
                            { // Slider
                                ImGui::Text("Fog Start");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::visuals::fog_start).c_str()).x + 7);
                                ImGui::Text(std::to_string((int)globals::visuals::fog_start).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##FogStart", &globals::visuals::fog_start, 0.0f, 1000.0f, "%.0f");
                            }
                            { // Slider
                                ImGui::Text("Fog End");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::visuals::fog_end).c_str()).x + 7);
                                ImGui::Text(std::to_string((int)globals::visuals::fog_end).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##FogEnd", &globals::visuals::fog_end, globals::visuals::fog_start + 1.0f, 2000.0f, "%.0f");
                            }
                        }

                        ImGui::Checkbox("Lock Indicator", &globals::visuals::lockedindicator);

                        ImGui::Checkbox("Crosshair##crosshair", &globals::visuals::crosshair_enabled);
                        if (globals::visuals::crosshair_enabled)
                        {
                            { // Slider
                                ImGui::Text("Size##crosshair");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::visuals::crosshair_size).substr(0, 3).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::visuals::crosshair_size).substr(0, 3).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##SizeCrosshair", &globals::visuals::crosshair_size, 1.0f, 50.0f, "%.1f");
                            }
                            { // Slider
                                ImGui::Text("Gap##crosshair");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::visuals::crosshair_gap).substr(0, 3).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::visuals::crosshair_gap).substr(0, 3).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##GapCrosshair", &globals::visuals::crosshair_gap, 0.0f, 30.0f, "%.1f");
                            }
                            { // Slider
                                ImGui::Text("Thickness##crosshair");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::visuals::crosshair_thickness).substr(0, 3).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::visuals::crosshair_thickness).substr(0, 3).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##ThicknessCrosshair", &globals::visuals::crosshair_thickness, 0.5f, 5.0f, "%.1f");
                            }
                            { // Slider
                                ImGui::Text("Speed##crosshair");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::visuals::crosshair_baseSpeed).c_str()).x + 7);
                                ImGui::Text(std::to_string((int)globals::visuals::crosshair_baseSpeed).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##SpeedCrosshair", &globals::visuals::crosshair_baseSpeed, 10.0f, 500.0f, "%.0f");
                            }
                            ImGui::Checkbox("Tween##crosshair", &globals::visuals::crosshair_gapTween);
                            if (globals::visuals::crosshair_gapTween)
                            { // Slider
                                ImGui::Text("Gap Speed##crosshair");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string(globals::visuals::crosshair_gapSpeed).substr(0, 3).c_str()).x + 7);
                                ImGui::Text(std::to_string(globals::visuals::crosshair_gapSpeed).substr(0, 3).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##GapSpeedCrosshair", &globals::visuals::crosshair_gapSpeed, 0.1f, 5.0f, "%.1f");
                            }
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::Combo("Style##crosshair", &globals::visuals::crosshair_styleIdx, "Static\0Pulse\0");
                        }


                        ImGui::Spacing();
                        ImGui::Separator();

                        // Waypoints section removed

                        ImGui::EndChild();

                        // Right-side Colors panel, like before
                        ImGui::SameLine();
                        ImGui::BeginChild("VisualsColors", ImVec2(0, 0), true);
                        // One control per line except sonar pair on the same line
                        ImGui::ColorEdit4("Boxes", globals::visuals::boxcolors, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Box Fill", globals::visuals::boxfillcolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Box Color 1", globals::visuals::box_gradient_color1, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Box Color 2", globals::visuals::box_gradient_color2, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Name", globals::visuals::namecolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Flags", globals::visuals::flagscolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Health Start", globals::visuals::healthbarcolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Health End", globals::visuals::healthbarcolor1, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Distance", globals::visuals::distancecolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Tool Name", globals::visuals::toolespcolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Skeletons", globals::visuals::skeletonscolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("China Hat", globals::visuals::chinahat_color, ImGuiColorEditFlags_NoInputs);

                        // Head Dot color removed
                        ImGui::ColorEdit4("Tracers", globals::visuals::tracerscolor, ImGuiColorEditFlags_NoInputs);

                        ImGui::ColorEdit4("Sonar Ring", globals::visuals::sonarcolor, ImGuiColorEditFlags_NoInputs);
                        ImGui::SameLine();
                        ImGui::ColorEdit4("Sonar Dot", globals::visuals::sonar_dot_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Crosshair", globals::visuals::crosshair_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Fog", globals::visuals::fog_color, ImGuiColorEditFlags_NoInputs);

                        ImGui::Checkbox("Chams Glow", &globals::visuals::chams_glow_s);
                        ImGui::Checkbox("Chams Fill", &globals::visuals::chams_highlight_s);
                        ImGui::ColorEdit4("Chams Color", globals::visuals::chams_outline, ImGuiColorEditFlags_NoInputs);
                        ImGui::SliderFloat("Chams Thickness", &globals::visuals::chams_thickness, 0.1, 3);
                        ImGui::EndChild();
                    }
                    else if (selected_tab == 5) // Movement tab
                    {
                        // Movement section - all movement options in one view
                        ImGui::Columns(2, nullptr, false);
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("MovementLeft", ImVec2(0, 0), true);
                        ImGui::TextDisabled("Movement Settings");
                        ImGui::Spacing();

                        // Speed
                        ImGui::Checkbox("Speed", &globals::misc::speed);
                        ImGui::SameLine();
                        float keybind_width_speed = ImMax(70.0f, ImGui::GetContentRegionAvail().x * 0.3f);
                        Bind(&globals::misc::speedkeybind, ImVec2(keybind_width_speed, 20.0f));
                        BeginVisualDisabled(!globals::misc::speed);
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
                        ImGui::Combo("Speed Type", &globals::misc::speedtype, "WalkSpeed\0CFrame\0");
                        { // Slider
                            ImGui::Text("Speed Value");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::misc::speedvalue).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::misc::speedvalue).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##SpeedValue", &globals::misc::speedvalue, 1.0f, 500.0f, "%.0f");
                        }
                        EndVisualDisabled();

                        ImGui::Spacing();

                        // Flight
                        ImGui::Checkbox("Flight", &globals::misc::flight);
                        ImGui::SameLine();
                        float keybind_width_flight = ImMax(70.0f, ImGui::GetContentRegionAvail().x * 0.3f);
                        Bind(&globals::misc::flightkeybind, ImVec2(keybind_width_flight, 20.0f));
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
                        ImGui::Combo("Flight Type", &globals::misc::flighttype, "CFrame\0Velocity\0Disabled\0");
                        { // Slider
                            ImGui::Text("Flight Speed");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::misc::flightvalue).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::misc::flightvalue).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##FlightValue", &globals::misc::flightvalue, 1.0f, 500.0f, "%.0f");
                        }

                        ImGui::EndChild();

                        ImGui::NextColumn();
                        ImGui::Spacing();  ImGui::Spacing();
                        ImGui::BeginChild("MovementRight", ImVec2(0, 0), true);
                        ImGui::TextDisabled("Advanced Movement");
                        ImGui::Spacing();

                        // 360 Spin
                        ImGui::Checkbox("360 Spin", &globals::misc::spin360);
                        ImGui::SameLine();
                        float keybind_width_spin = ImMax(70.0f, ImGui::GetContentRegionAvail().x * 0.3f);
                        Bind(&globals::misc::spin360keybind, ImVec2(keybind_width_spin, 20.0f));
                        BeginVisualDisabled(!globals::misc::spin360);
                        { // Slider
                            ImGui::Text("Spin Speed");
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::misc::spin360speed).c_str()).x + 7);
                            ImGui::Text(std::to_string((int)globals::misc::spin360speed).c_str());
                            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                            ImGui::SliderFloat("##SpinSpeed", &globals::misc::spin360speed, 1.0f, 10.0f, "%.1f");
                        }
                        EndVisualDisabled();

                        // Vehicle Fly (moved from player tab)
                        ImGui::Checkbox("Vehicle Fly", &globals::misc::bikefly);

                        ImGui::EndChild();
                    }
                    else if (selected_tab == 6) // Menu tab
                    {
                        ImGui::Checkbox("Custom Menu Keybind", &g_menu_custom_bind_enabled);
                        ImGui::SameLine();
                        if (g_menu_custom_bind_enabled) {
                            Bind(&g_menu_toggle_keybind, ImVec2(100.0f, 22.0f));
                        }
                        else {
                            ImGui::BeginDisabled();
                            ImGui::Button("Insert", ImVec2(100.0f, 22.0f));
                            ImGui::EndDisabled();
                        }

                        ImGui::Checkbox("VSYNC", &globals::misc::vsync);
                        ImGui::Checkbox("Override Overlay FPS", &globals::misc::override_overlay_fps);
                        if (globals::misc::override_overlay_fps) {
                            { // Slider
                                ImGui::Text("FPS##overlay");
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(std::to_string((int)globals::misc::overlay_fps).c_str()).x + 7);
                                ImGui::Text(std::to_string((int)globals::misc::overlay_fps).c_str());
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::SliderFloat("##FPSOverlay", &globals::misc::overlay_fps, 15.0f, 3000.0f, "%.0f");
                            }
                        }
                        ImGui::Checkbox("Streamproof", &globals::misc::streamproof);

                        // Rescan button
                        if (ImGui::Button("Rescan Game", ImVec2(120.0f, 25.0f))) {
                            force_rescan();
                        }

                        std::vector<std::string> fontNames;
                        fontNames.emplace_back("Default");
                        for (const auto& f : g_fontFiles) {
                            fontNames.emplace_back(std::filesystem::path(f).filename().string());
                        }
                        std::string itemsJoined;
                        for (size_t i = 0; i < fontNames.size(); ++i) {
                            itemsJoined += fontNames[i];
                            itemsJoined.push_back('\0');
                        }
                        itemsJoined.push_back('\0');
                        ImGui::Combo("Active Font", &g_selectedFont, itemsJoined.c_str());
                        ImGui::SameLine();
                        if (ImGui::Button("Apply##font")) {
                            g_pendingFontReload = true;
                        }




                    }
                    else if (selected_tab == 7) // Config tab
                    {
                        ImVec2 avail = ImGui::GetContentRegionAvail();
                        g_config_system.render_config_ui((int)avail.x, (int)avail.y);
                    }


                    ImGui::EndChild(); // End ContentArea
                }
                ImGui::End();

                ImGui::PopStyleVar();

            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            overlay::visible = false;
        }

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("esp", NULL,
            ImGuiWindowFlags_NoBackground
            |
            ImGuiWindowFlags_NoResize
            |
            ImGuiWindowFlags_NoCollapse
            |
            ImGuiWindowFlags_NoTitleBar
            |
            ImGuiWindowFlags_NoInputs
            |
            ImGuiWindowFlags_NoMouseInputs
            |
            ImGuiWindowFlags_NoDecoration
            |
            ImGuiWindowFlags_NoMove); {

            globals::instances::draw = ImGui::GetBackgroundDrawList();
            // Snow background effect removed
            globals::in_render_loop = true;

            try {
                visuals::run();
            }
            catch (...) {
                printf("Overlay: visuals::run threw, skipping frame\n");
                globals::in_render_loop = false;
            }
            globals::in_render_loop = false;



            // No custom images

        }

        ImGui::End();


        if (overlay::visible) {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
        else
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

        }

        if (globals::misc::streamproof)
        {
            SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
        }
        else
        {
            SetWindowDisplayAffinity(hwnd, WDA_NONE);
        }



        Notifications::Update();
        Notifications::Render();





        // Safe font reload just before rendering to avoid hot-reload glitches
        if (g_pendingFontReload) {
            ImGuiIO& io2 = ImGui::GetIO();
            io2.Fonts->Clear();
            ImFontConfig cfg; cfg.OversampleH = 1; cfg.OversampleV = 1; cfg.PixelSnapH = true;
            if (g_selectedFont == 0) {
                io2.Fonts->AddFontDefault();
            }
            else if ((size_t)g_selectedFont <= g_fontFiles.size()) {
                io2.Fonts->AddFontDefault();
                const std::string& selPath = g_fontFiles[(size_t)g_selectedFont - 1];
                io2.Fonts->AddFontFromFileTTF(selPath.c_str(), 13.0f, &cfg, io2.Fonts->GetGlyphRangesDefault());
            }
            io2.Fonts->Build();
            // Recreate device objects to ensure new font atlas is uploaded
            ImGui_ImplDX11_InvalidateDeviceObjects();
            ImGui_ImplDX11_CreateDeviceObjects();
            g_pendingFontReload = false;
        }

        // Render desync timer UI
        render_desync_timer();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present with optional FPS override
        if (globals::misc::override_overlay_fps && globals::misc::overlay_fps > 0.0f) {
            static double accumulator = 0.0;
            static LARGE_INTEGER freq = { 0 };
            static LARGE_INTEGER last = { 0 };
            if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
            LARGE_INTEGER now; QueryPerformanceCounter(&now);
            double dt = (last.QuadPart == 0) ? 0.0 : double(now.QuadPart - last.QuadPart) / double(freq.QuadPart);
            last = now;
            accumulator += dt;
            const double target_dt = 1.0 / std::max(1.0f, globals::misc::overlay_fps);
            if (accumulator < target_dt) {
                double sleep_s = target_dt - accumulator;
                DWORD sleep_ms = (DWORD)std::max(0.0, sleep_s * 1000.0);
                if (sleep_ms > 0) Sleep(sleep_ms);
            }
            accumulator = 0.0;
            g_pSwapChain->Present(0, 0);
        }
        else {
            if (globals::misc::vsync) g_pSwapChain->Present(1, 0);
            else g_pSwapChain->Present(0, 0);
        }

        // Removed overlay loaded popup
        // Damage indicators removed

      /*  static LARGE_INTEGER frequency;
        static LARGE_INTEGER lastTime;
        static bool timeInitialized = false;

        if (!timeInitialized) {
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&lastTime);
            timeBeginPeriod(1);
            timeInitialized = true;
        }

        const double targetFrameTime = 1.0 / 165;

        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        double elapsedSeconds = static_cast<double>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;

        if (elapsedSeconds < targetFrameTime) {
            DWORD sleepMilliseconds = static_cast<DWORD>((targetFrameTime - elapsedSeconds) * 1000.0);
            if (sleepMilliseconds > 0) {
                Sleep(sleepMilliseconds);
            }
        }

        do {
            QueryPerformanceCounter(&currentTime);
            elapsedSeconds = static_cast<double>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        } while (elapsedSeconds < targetFrameTime);

        lastTime = currentTime;*/

        if (overlay::visible) {
            // Overlay visibility logic here
        }
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanup_avatar_system();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

bool fullsc(HWND windowHandle)
{
    MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
    if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
    {
        RECT rect;
        if (GetWindowRect(windowHandle, &rect))
        {
            return rect.left == monitorInfo.rcMonitor.left
                && rect.right == monitorInfo.rcMonitor.right
                && rect.top == monitorInfo.rcMonitor.top
                && rect.bottom == monitorInfo.rcMonitor.bottom;
        }
    }
    return false;
}

void movewindow(HWND hw) {
    HWND target = FindWindowA(0, "Roblox");
    HWND foregroundWindow = GetForegroundWindow();

    if (target != foregroundWindow && hw != foregroundWindow)
    {
        MoveWindow(hw, 0, 0, 0, 0, true);
    }
    else
    {
        RECT rect;
        GetWindowRect(target, &rect);

        int rsize_x = rect.right - rect.left;
        int rsize_y = rect.bottom - rect.top;

        if (fullsc(target))
        {
            rsize_x += 16;
            rect.right -= 16;
        }
        else
        {
            rsize_y -= 39;
            rect.left += 8;
            rect.top += 31;
            rect.right -= 16;
        }
        rsize_x -= 16;
        MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
    }
}
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();

    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}