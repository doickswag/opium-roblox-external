#pragma once
#include "../util/classes/classes.h"
#include "../drawing/overlay/keybind/keybind.h"
#include <unordered_map>

namespace globals
{
	inline bool focused;
	inline bool firstreceived = false;
	inline bool unattach = false;
	inline bool handlingtp = false;
	inline std::vector<roblox::wall> walls;

	namespace bools {
		inline bool autokill, kill, bring;
		inline std::string name;
		inline roblox::player entity;
	}

	inline bool in_render_loop = false;
	inline std::unordered_map<uint64_t, bool> teammate_cache;
	inline std::unordered_map<uint64_t, bool> teammate_last_verdict;
	inline std::unordered_map<uint64_t, int> teammate_verdict_streak;

	namespace instances {
		inline std::vector<std::string> whitelist;
		inline std::vector<std::string> blacklist;
		inline roblox::instance visualengine;
		inline roblox::instance datamodel;
		inline roblox::instance workspace;
		inline roblox::instance players;
		inline roblox::player lp;
		inline roblox::instance lighting;
		inline roblox::camera camera;
		inline roblox::instance localplayer;
		inline std::string gamename = "Universal";
		inline std::string username = "layuh";
		inline std::vector<roblox::player> cachedplayers;
		inline roblox::player cachedtarget;
		inline roblox::player cachedlasttarget;
		inline roblox::instance aim;
		inline uintptr_t mouseservice;
		inline ImDrawList* draw;
	}

	namespace combat {
		inline bool rapidfire;
		inline bool autostomp;
		inline bool aimbot = false;
		inline bool stickyaim = true;
		inline bool unlockondeath = false;
		inline int aimbottype = 0;
		inline keybind aimbotkeybind("aimbotkeybind");
		inline bool usefov = false;
		inline bool drawfov = false;
		inline float fovsize = 50;
		inline float fovcolor[4] = { 1,1,1,1 };
		inline bool smoothing = false;
		inline float smoothingx = 5;
		inline float smoothingy = 5;
		inline int smoothingstyle = 1;
		inline bool sensitivity_enabled = true;
		inline float cam_sensitivity = 1.0f;
		inline bool camlock_shake = false;
		inline float camlock_shake_x = 2.0f;
		inline float camlock_shake_y = 2.0f;
		inline float mouse_sensitivity = 1.0f;
		inline bool predictions = false;
		inline float predictionsx = 5;
		inline float predictionsy = 5;
		inline bool deadzone = false;
		inline float deadzonex = 0.0f;
		inline float deadzoney = 0.0f;
		inline bool autoswitch = false;
		inline bool teamcheck = false;
		inline bool grabbedcheck = false;
		inline bool knockcheck = false;
		inline bool rangecheck = false;
		inline bool healthcheck = false;
		inline bool wallcheck = false;
		inline bool arsenal_flick_fix = false;
		inline std::vector<int>* flags = new std::vector<int>{ 0, 0, 0, 0, 0, 0, 0 };
		inline float range = 1000;
		inline float healththreshhold = 10;
		inline int aimpart = 0;
		inline bool silentaim = false;
		inline bool stickyaimsilent = true;
		inline bool connect_to_aimbot = false;
		inline bool aimbot_locked = false;
		inline roblox::player aimbot_current_target;
		inline int hitchance = 100;
		inline keybind silentaimkeybind("silentaimkeybind");
		inline bool spoofmouse = true;
		inline int closestpartsilent = 0;
		inline int silentaimpart = 0;
		inline int mouselockpart = 0;
		inline bool nosleep_aimbot = false;
		inline int camlockpart = 0;
		inline bool airpart_enabled = false;
		inline int airpart = 0;
		inline int target_method = 0;
		inline bool usesfov = false;
		inline bool drawsfov = false;
		inline float sfovsize = 50;
		inline float sfovcolor[4] = { 1,1,1,1 };
		inline bool silentpredictions = false;
		inline float silentpredictionsx = 5;
		inline float silentpredictionsy = 5;
		inline bool orbit = false;
		inline std::vector<int> orbittype = std::vector<int>(3, 0);
		inline float orbitspeed = 8;
		inline float orbitrange = 3;
		inline float orbitheight = 1;
		inline bool drawradiusring = false;
		inline keybind orbitkeybind("orbitkeybind                 ");
		inline bool triggerbot = false;
		inline bool antiaim = false;
		inline bool underground_antiaim = false;
		inline float delay = 0;
		inline keybind triggerbotkeybind("triggerbotkeybind");
	}

	namespace visuals {
		inline bool visuals = true;
		inline bool boxes = false;
		inline bool lockedindicator = false;
		inline bool glowesp = false;
		inline int boxtype = 0;
		inline bool health = false;
		inline bool healthbar = false;
		inline bool armorbar = false;
		inline bool healthtext = false;
		inline bool chams = true;
		inline bool wireframe_chams = false;
		inline float wireframe_thickness = 1.5f;
		inline float wireframe_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline bool chams_glow_s = false;
		inline bool chams_highlight_s = false;
		inline bool chams_outline_s = false;
		inline float chams_thickness = 1.0f;
		inline float chamscolor[4] = { 1.0f, 1.0f, 1.0, 0.5f };
		inline float chams_fillcolor[4] = { 1.0f, 1.0f, 1.0f, 0.4f };
		inline float chams_glowcolor[4] = { 1.0f, 1.0f, 1.0f, 0.7f };
		inline float chams_outline[4] = { 1.0f, 1.0f, 1.0f, 0.7f };
		inline bool name = false;
		inline int nametype = 0;
		inline bool toolesp = false;
		inline bool distance = false;
		inline bool flags = false;
		inline bool skeletons = false;
		inline bool selfesp = false;
		inline bool maxdistance_enabled = false;
		inline float maxdistance = 1000.0f;
		inline bool chinahat = false;
		inline bool chinahat_target_only = false;
		inline float chinahat_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline bool localplayer = false;
		inline bool predictionsdot = false;
		inline bool radar = false;
		inline float radar_size = 160.0f;
		inline float radar_range = 250.0f;
		inline float radar_pos[2] = { 20.0f, 20.0f };
		inline bool radar_rotate = true;
		inline bool radar_show_distance = true;
		inline bool radar_outline = true;
		inline float radar_background[4] = { 0.06f, 0.06f, 0.06f, 0.6f };
		inline float radar_border[4] = { 0.0f, 0.0f, 0.0f, 0.8f };
		inline float radar_dot[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float boxcolors[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float boxfillcolor[4] = { 0.2f, 0.2f, 0.2f, 0.3f };
		inline float box_gradient_color1[4] = { 0.8f, 0.2f, 0.8f, 0.5f };
		inline float box_gradient_color2[4] = { 0.2f, 0.8f, 0.8f, 0.5f };
		inline float glowcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline bool box_gradient = false;
		inline bool box_gradient_rotation = false;
		inline float box_gradient_rotation_speed = 0.25f;
		inline float lockedcolor[4] = { 1,0,0,1 };
		inline float healthbarcolor[4] = { 0,1,0,1 };
		inline float healthbarcolor1[4] = { 1,0,0,1 };
		inline float namecolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float toolespcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float distancecolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float sonarcolor[4] = { 1.0f, 1.0f, 1.0f, 0.8f };
		inline float sonar_dot_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		inline float skeletonscolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline bool sonar = false;
		inline float sonar_range = 50.0f;
		inline float sonar_thickness = 1.0f;
		inline bool sonar_show_distance = false;
		inline bool teams = false;
		inline float teamscolor[3] = { 0.3647f, 0.4471f, 0.4549f };
		inline std::vector<int>* team_overlay_flags = new std::vector<int>{ 0, 0 };
		inline bool fog = false;
		inline float fog_start = 0.0f;
		inline float fog_end = 1000.0f;
		inline float fog_color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		inline bool fortniteindicator = false;
		inline bool hittracer = false;
		inline bool trail = false;
		inline float trail_duration = 3.0f;
		inline float trail_color[4] = { 1.0f, 1.0f, 1.0f, 0.8f };
		inline float trail_thickness = 2.0f;
		inline bool hitbubble = false;
		inline bool targetskeleton = false;
		inline bool tracers = false;
		inline int tracerstype = 0;
		inline bool tracers_outline = false;
		inline bool tracers_glow = false;
		inline float tracerscolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float flagscolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float flags_font_size = 11.0f;
		inline float flags_offset_x = 5.0f;
		inline bool enemycheck = true;
		inline bool friendlycheck = true;
		inline bool teamcheck = false;
		inline bool rangecheck = false;
		inline float range = 1000;
		inline bool crosshair_enabled = false;
		inline float crosshair_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline float crosshair_size = 10.0f;
		inline float crosshair_gap = 5.0f;
		inline bool crosshair_gapTween = false;
		inline float crosshair_gapSpeed = 1.0f;
		inline float crosshair_thickness = 1.0f;
		inline int crosshair_styleIdx = 0;
		inline float crosshair_baseSpeed = 100.0f;
		inline float crosshair_fadeDuration = 1.0f;
		inline std::vector<int>* box_overlay_flags = new std::vector<int>{ 0, 0, 0 };
		inline std::vector<int>* name_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* flag_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* healthbar_overlay_flags = new std::vector<int>{ 0, 0, 0 };
		inline std::vector<int>* tool_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* distance_overlay_flags = new std::vector<int>{ 0, 0 };
		inline std::vector<int>* skeleton_overlay_flags = new std::vector<int>{ 0, 0 };
	}

	namespace misc {
		inline bool speed = false;
		inline int speedtype = 0;
		inline float speedvalue = 16;
		inline keybind speedkeybind("speedkeybind");
		inline bool custom_model_enabled = false;
		inline std::string custom_model_folder_path = "";
		inline std::mutex custom_model_mutex;
		inline bool spin360 = false;
		inline float spin360speed = 5.0f;
		inline keybind spin360keybind("spin360keybind");
		inline bool jumppower = false;
		inline float jumpowervalue = 50;
		inline keybind jumppowerkeybind("jumppowerkeybind");
		inline bool voidhide = false;
		inline keybind voidhidebind("voidehidkeybind");
		inline bool flight = false;
		inline int flighttype = 0;
		inline float flightvalue = 16;
		inline keybind flightkeybind("flightkeybind");
		inline bool hipheight = false;
		inline float hipheightvalue = 16;
		inline bool dahood_rapidfire = false;
		inline bool arsenal_rapidfire = false;
		inline float rapid_fire_value = 0.f;
		inline bool autoarmor = false;
		inline bool autoreload = false;
		inline bool autostomp = false;
		inline bool antistomp = false;
		inline bool bikefly = false;
		inline keybind stompkeybind("stompkeybind");
		inline bool animation = false;
		inline int animationtype = 0;
		inline bool cframe = false;
		inline int cframespeed = 16;
		inline keybind cframekeybind("cframekeybind");
		inline bool antiaim = false;
		inline bool desync = false;
		inline keybind desynckeybind("desynckeybind");
		inline bool desync_active = false;
		inline float desync_timer = 0.0f;
		inline const float desync_max_time = 15.0f;
		inline bool desync_visualizer = false;
		inline float desync_viz_color[4] = { 1.0f, 0.0f, 0.0f, 0.8f };
		inline const float desync_viz_range = 3.0f;
		inline const float desync_viz_thickness = 2.0f;
		inline Vector3 desync_activation_pos = Vector3(0, 0, 0);
		inline bool spam_tp = false;
		inline std::string roblox_path = "";
		inline std::string firewall_rule_name = "Calendar";
		inline bool spectate = false;
		inline keybind spectatebind("spectatebind");
		inline bool vsync = true;
		inline int keybindsstyle;
		inline bool targethud = false;
		inline bool playerlist = true;
		inline bool streamproof = false;
		inline bool keybinds = false;
		inline bool spotify = false;
		inline bool override_overlay_fps = false;
		inline float overlay_fps = 60.0f;
		inline bool colors = false;
		inline bool wallcheck = false;
	}

	inline bool is_teammate(const roblox::player& player) {
		// First grab the local player's team
		std::string localTeam = globals::instances::lp.getTeam();
		
		// Then check if the current player's team matches our local player's team
		if (player.getTeam() == localTeam && !localTeam.empty() && localTeam != "Unknown") {
			return true; // They are teammates
		}
		
		return false; // They are not teammates
	}

	inline bool is_grabbed(const roblox::player& player) {
		if (!is_valid_address(player.main.address)) return false;
		auto grabber = player.main.findfirstchild("Grabber");
		if (is_valid_address(grabber.address)) return true;
		auto grabbed = player.main.findfirstchild("Grabbed");
		if (is_valid_address(grabbed.address)) return true;
		return false;
	}
}