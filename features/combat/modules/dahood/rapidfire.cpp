#include <thread>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <mmsystem.h>
#include <vector>
#include <memory>
#include <thread>
#include <iostream>
#include "../../combat.h"
#include "../../../hook.h"

void hooks::dahood_rapidfire() {
    while (true) {
        if (!globals::focused || !globals::misc::dahood_rapidfire) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        if (globals::focused && globals::misc::dahood_rapidfire) {
            roblox::instance backpack = globals::instances::lp.main.findfirstchild("Backpack");
            for (roblox::instance tool : backpack.get_children()) {
                if (tool.get_class_name() != "Tool" && tool.get_name() != "Combat") continue;
                roblox::instance shooting = tool.findfirstchild("ShootingCooldown");
                roblox::instance tyolerance = tool.findfirstchild("ToleranceCooldown");
                shooting.write_double_value(0.000000001);
                tyolerance.write_double_value(0.000000001);
            }
        }
    }
}

void hooks::arsenal_rapidfire() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (!globals::misc::arsenal_rapidfire) {
            continue;
        }

        try {
            auto replicatedStorage = globals::instances::datamodel.findfirstchild("ReplicatedStorage");
            if (!replicatedStorage.is_valid() || replicatedStorage.address == 0) {
                continue;
            }

            auto weaponsFolder = replicatedStorage.findfirstchild("Weapons");
            if (!weaponsFolder.is_valid() || weaponsFolder.address == 0) {
                continue;
            }

            auto weaponChildren = weaponsFolder.get_children();

            for (auto& weapon : weaponChildren) {
                if (!weapon.is_valid() || weapon.address == 0) {
                    continue;
                }
                auto fireRateValue = weapon.findfirstchild("FireRate");
                if (fireRateValue.is_valid() && fireRateValue.address != 0) {
                    fireRateValue.write_float_value(globals::misc::rapid_fire_value);
                }
            }
        }
        catch (...) {
        }
    }
}