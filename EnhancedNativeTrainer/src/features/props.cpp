/*
Part of the Enhanced Native Trainer project.
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham and fellow contributors 2015
*/

#include "props.h"
#include "script.h"

int lastSelectedCategoryIndex = 0;

const std::vector<std::string> PROP_CATEGORIES =
{
	"Y",
	"Z",
	"Agricultural",
	"Aviation",
	"Bags",
	"Bars & Clubs",
	"Bathroom",
	"Beach",
	"Bins",
	"Boxes & Crates",
	"Building Infrastructure",
	"Cars",
	"Chairs",
	"Clocks",
	"Clothing",
	"Computers",
	"Construction",
	"Doors",
	"Drink",
	"Drugs",
	"Fairground",
	"Fences",
	"Film Studio",
	"Firing Range",
	"Food & Drink",
	"Freight",
	"Garage",
	"Garden",
	"Gates",
	"Golf Course",
	"Heist Photos",
	"Household",
	"Icons",
	"Industrial",
	"Jewellery Store",
	"Junk",
	"Kitchen",
	"Leisure",
	"Marine",
	"Military",
	"Misc",
	"Mobile Phones",
	"Money",
	"Nature",
	"Office",
	"Paint",
	"Papers",
	"Personal Effects",
	"Police",
	"Pool Table",
	"Railway",
	"Ramps",
	"Roads",
	"Ropes & Hooks",
	"Shopping",
	"Signs",
	"Sports Equipment",
	"Street Furniture",
	"Tables",
	"TV & Audio",
	"Tools",
	"Weaponry"
};

//DO NOT EDIT THESE DIRECTLY, USE THE SPREADSHEET IN DOCUMENTS & EXPORT FROM THERE
const std::vector<PropInfo> ALL_PROPS =
{
};

bool param1 = true;
bool param2 = true;
bool param3 = true;

float vectRads(float degs)
{
	float radialConv = degs*3.1415926536 / 180;
	return radialConv;
}

std::string lastCustomPropSpawn;

void do_spawn_model(Hash propHash, char* model, std::string title, bool silent)
{
	STREAMING::REQUEST_MODEL(propHash);
	DWORD now = GetTickCount();
	while (!STREAMING::HAS_MODEL_LOADED(propHash) && GetTickCount() < now + 5000 )
	{
		//make_periodic_feature_call();
		WAIT(0);
	}

	if (!STREAMING::HAS_MODEL_LOADED(propHash))
	{
		std::ostringstream ss2;
		ss2 << "TIMEOUT: " << model;
		write_text_to_log_file(ss2.str());
		return;
	}

	Ped playerPed = PLAYER::PLAYER_PED_ID();
	FLOAT look = ENTITY::GET_ENTITY_HEADING(playerPed);
	FLOAT lookAni = look + 180.00;
	FLOAT lookOff = look + 90.00;
	FLOAT vecX = 0;
	FLOAT vecY = 0;

	FLOAT spawnOffX = 0.0f;
	FLOAT spawnOffY = 3.5f;
	FLOAT spawnOffZ = 4.0f;

	Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, spawnOffX, spawnOffY, spawnOffZ);

	Object obj = OBJECT::CREATE_OBJECT_NO_OFFSET(propHash, coords.x, coords.y, coords.z, param1, param2, param3);

	if (ENTITY::DOES_ENTITY_EXIST(obj))
	{
		ENTITY::SET_ENTITY_COLLISION(obj, 1, 0);
		OBJECT::PLACE_OBJECT_ON_GROUND_PROPERLY(obj);
		ENTITY::SET_ENTITY_HAS_GRAVITY(obj, true);
		ENTITY::FREEZE_ENTITY_POSITION(obj, false);
		OBJECT::SET_ACTIVATE_OBJECT_PHYSICS_AS_SOON_AS_IT_IS_UNFROZEN(obj, true);
		ENTITY::SET_ENTITY_LOAD_COLLISION_FLAG(obj, true);
	}
	else
	{
		if (!silent)
		{
			std::ostringstream ss;
			ss << "Failed to create " << title;
			set_status_text(ss.str());
		}

		std::ostringstream ss2;
		ss2 << "INVALID-PROP: " << model;
		write_text_to_log_file(ss2.str());
		return;
	}

	if (!silent)
	{
		std::ostringstream ss;
		ss << "Spawned " << title;
		set_status_text(ss.str());
	}

	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(propHash);
	ENTITY::SET_OBJECT_AS_NO_LONGER_NEEDED(&obj);
}

void do_spawn_prop(PropInfo prop, bool silent)
{

	Hash propHash = GAMEPLAY::GET_HASH_KEY((char *)prop.model);

	if (!STREAMING::IS_MODEL_IN_CDIMAGE(propHash) || !STREAMING::IS_MODEL_VALID(propHash))
	{
		if (!silent)
		{
			std::ostringstream ss;
			ss << "Model " << prop.model << " is not valid";
			set_status_text(ss.str());
		}

		std::ostringstream ss2;
		ss2 << "INVALID-MODEL: " << prop.model;
		write_text_to_log_file(ss2.str());
		return;
	}

	do_spawn_model(propHash, prop.model, prop.label, silent);
}

bool onconfirm_prop_selection(MenuItem<int> choice)
{
	std::string category = PROP_CATEGORIES[lastSelectedCategoryIndex];

	std::vector<PropInfo> filtered;
	for each (PropInfo prop in ALL_PROPS)
	{
		if (prop.category.compare(category) == 0)
		{
			filtered.push_back(prop);
		}
	}

	if (choice.value == -1)
	{
		int i = 0;
		for each (PropInfo prop  in filtered)
		{
			std::ostringstream ss;
			ss << "Done " << i++ << " of " << filtered.size();
			set_status_text_centre_screen(ss.str());
			WAIT(0);

			do_spawn_prop(prop, true);
		}
		return false;
	}

	PropInfo prop = filtered.at(choice.value);
	do_spawn_prop(prop, false);

	return false;
}

int propCategorySelection = 0;
int propSelection = 0;

void process_props_menu_incategory(int categoryIndex)
{
	std::string category = PROP_CATEGORIES[categoryIndex];

	std::vector<PropInfo> filtered;
	int count = 0;
	int total = 0;
	for each (PropInfo prop in ALL_PROPS)
	{
		total++;
		if (prop.category.compare(category) == 0)
		{
			count++;
			filtered.push_back(prop);
		}
	}

	std::ostringstream ssd;
	ssd << count << " of " << total << " and " << ALL_PROPS.size();
	set_status_text_centre_screen(ssd.str());

	std::vector<MenuItem<int>*> menuItems;

	MenuItem<int>* item = new MenuItem<int>();
	item->value = -1;
	item->caption = "Spawn All In Category";
	item->isLeaf = true;
	menuItems.push_back(item);

	int i = 0;
	for each (PropInfo prop in filtered)
	{
		MenuItem<int>* item = new MenuItem<int>();
		item->value = i;
		item->caption = prop.label;
		item->isLeaf = true;
		menuItems.push_back(item);
		i++;
	}

	draw_generic_menu<int>(menuItems, &propSelection, category, onconfirm_prop_selection, NULL, NULL, NULL);
}

bool onconfirm_prop_category(MenuItem<int> choice)
{
	if (choice.value == -1)
	{
		std::string result = show_keyboard(NULL, (char*)lastCustomPropSpawn.c_str());
		if (!result.empty())
		{
			result = trim(result);
			lastCustomPropSpawn = result;
			Hash hash = GAMEPLAY::GET_HASH_KEY((char*)result.c_str());
			if (!STREAMING::IS_MODEL_IN_CDIMAGE(hash) || !STREAMING::IS_MODEL_VALID(hash))
			{
				std::ostringstream ss;
				ss << "Couldn't find model '" << result << "'";
				set_status_text(ss.str());
				return false;
			}
			else
			{
				do_spawn_model(GAMEPLAY::GET_HASH_KEY((char*)result.c_str()), (char*)result.c_str(), result, false);
			}
		}
		return false;
	}

	if (choice.value != lastSelectedCategoryIndex)
	{
		propSelection = 0;
		lastSelectedCategoryIndex = choice.value;
	}
	process_props_menu_incategory(choice.value);
	return false;
}

void process_props_menu()
{
	std::vector<MenuItem<int>*> menuItems;

	int i = 0;
	for each (std::string category in PROP_CATEGORIES )
	{
		MenuItem<int>* item = new MenuItem<int>();
		item->value = i;
		item->caption = category;
		item->isLeaf = false;
		menuItems.push_back(item);
		i++;
	}

	MenuItem<int>* item = new MenuItem<int>();
	item->value = -1;
	item->caption = "Enter Name Manually";
	item->isLeaf = false;
	menuItems.push_back(item);
	i++;

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "Spawn Param 1";
	toggleItem->toggleValue = &param1;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "Spawn Param 2";
	toggleItem->toggleValue = &param2;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "Spawn Param 3";
	toggleItem->toggleValue = &param3;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &propCategorySelection, "Prop Categories", onconfirm_prop_category, NULL, NULL, NULL);

}