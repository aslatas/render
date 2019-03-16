#pragma once

struct PhysicsSettings
{
};

PhysicsSettings* LoadPhysicsSettings(char* filename);
void SavePhysicsSettings(char* filename);
void FreePhysicsSettings(PhysicsSettings* physics_settings);