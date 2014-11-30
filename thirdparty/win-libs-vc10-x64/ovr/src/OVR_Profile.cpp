/************************************************************************************

PublicHeader:   None
Filename    :   OVR_Profile.cpp
Content     :   Structs and functions for loading and storing device profile settings
Created     :   February 14, 2013
Notes       :
   
   Profiles are used to store per-user settings that can be transferred and used
   across multiple applications.  For example, player IPD can be configured once 
   and reused for a unified experience across games.  Configuration and saving of profiles
   can be accomplished in game via the Profile API or by the official Oculus Configuration
   Utility.

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "OVR_Profile.h"
#include "OVR_JSON.h"
#include "Kernel/OVR_Types.h"
#include "Kernel/OVR_SysFile.h"
#include "Kernel/OVR_Allocator.h"
#include "Kernel/OVR_Array.h"

#ifdef OVR_OS_WIN32
#include <Shlobj.h>
#else
#include <dirent.h>
#include <sys/stat.h>

#ifdef OVR_OS_LINUX
#include <unistd.h>
#include <pwd.h>
#endif

#endif

#define PROFILE_VERSION 1.0

namespace OVR {

//-----------------------------------------------------------------------------
// Returns the pathname of the JSON file containing the stored profiles
String GetBaseOVRPath(bool create_dir)
{
    String path;

#if defined(OVR_OS_WIN32)

    TCHAR data_path[MAX_PATH];
    SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, NULL, 0, data_path);
    path = String(data_path);
    
    path += "/Oculus";

    if (create_dir)
    {   // Create the Oculus directory if it doesn't exist
        WCHAR wpath[128];
        OVR::UTF8Util::DecodeString(wpath, path.ToCStr());

        DWORD attrib = GetFileAttributes(wpath);
        bool exists = attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY);
        if (!exists)
        {   
            CreateDirectory(wpath, NULL);
        }
    }
        
#elif defined(OVR_OS_MAC)

    const char* home = getenv("HOME");
    path = home;
    path += "/Library/Preferences/Oculus";

    if (create_dir)
    {   // Create the Oculus directory if it doesn't exist
        DIR* dir = opendir(path);
        if (dir == NULL)
        {
            mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
        }
        else
        {
            closedir(dir);
        }
    }

#else

    passwd* pwd = getpwuid(getuid());
    const char* home = pwd->pw_dir;
    path = home;
    path += "/.config/Oculus";

    if (create_dir)
    {   // Create the Oculus directory if it doesn't exist
        DIR* dir = opendir(path);
        if (dir == NULL)
        {
            mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
        }
        else
        {
            closedir(dir);
        }
    }

#endif

    return path;
}

String GetProfilePath(bool create_dir)
{
    String path = GetBaseOVRPath(create_dir);
    path += "/Profiles.json";
    return path;
}

//-----------------------------------------------------------------------------
// ***** ProfileManager

ProfileManager::ProfileManager()
{
    Changed = false;
    CacheDevice = Profile_Unknown;
}

ProfileManager::~ProfileManager()
{
    // If the profiles have been altered then write out the profile file
    if (Changed)
        SaveCache();

    ClearCache();
}

ProfileManager* ProfileManager::Create()
{
    return new ProfileManager();
}

Profile* ProfileManager::CreateProfileObject(const char* user,
                                             ProfileType device,
                                             const char** device_name)
{
    Lock::Locker lockScope(&ProfileLock);

    Profile* profile = NULL;
    switch (device)
    {
        case Profile_RiftDK1:
            *device_name = "RiftDK1";
            profile = new RiftDK1Profile(user);
            break;
        case Profile_RiftDKHD:
        case Profile_Unknown:
            break;
    }

    return profile;
}


// Clear the local profile cache
void ProfileManager::ClearCache()
{
    Lock::Locker lockScope(&ProfileLock);

    ProfileCache.Clear();
    CacheDevice = Profile_Unknown;
}


// Poplulates the local profile cache.  This occurs on the first access of the profile
// data.  All profile operations are performed against the local cache until the
// ProfileManager is released or goes out of scope at which time the cache is serialized
// to disk.
void ProfileManager::LoadCache(ProfileType device)
{
    Lock::Locker lockScope(&ProfileLock);

    ClearCache();

    String path = GetProfilePath(false);

    Ptr<JSON> root = *JSON::Load(path);
    if (!root || root->GetItemCount() < 3)
        return;

    // First read the file type and version to make sure this is a valid file
    JSON* item0 = root->GetFirstItem();
    JSON* item1 = root->GetNextItem(item0);
    JSON* item2 = root->GetNextItem(item1);

    if (OVR_strcmp(item0->Name, "Oculus Profile Version") == 0)
    {   // In the future I may need to check versioning to determine parse method
    }
    else
    {
        return;
    }

    DefaultProfile = item1->Value;

    // Read the number of profiles
    int   profileCount = (int)item2->dValue;
    JSON* profileItem  = item2;

    for (int p=0; p<profileCount; p++)
    {
        profileItem = profileItem->GetNextItem(profileItem);
        if (!profileItem)
            break;
        
        // Read the required Name field
        const char* profileName;
        JSON* item = profileItem->GetFirstItem();
        
        if (item && (OVR_strcmp(item->Name, "Name") == 0))
        {   
            profileName = item->Value;
        }
        else
        {
            return;   // invalid field
        }

        const char*   deviceName  = 0;
        bool          deviceFound = false;
        Ptr<Profile>  profile     = *CreateProfileObject(profileName, device, &deviceName);

        // Read the base profile fields.
        if (profile)
        {
            while (item = profileItem->GetNextItem(item), item)
            {
                if (item->Type != JSON_Object)
                {
                    profile->ParseProperty(item->Name, item->Value);
                }
                else
                {   // Search for the matching device to get device specific fields
                    if (!deviceFound && OVR_strcmp(item->Name, deviceName) == 0)
                    {
                        deviceFound = true;

                        for (JSON* deviceItem = item->GetFirstItem(); deviceItem;
                             deviceItem = item->GetNextItem(deviceItem))
                        {
                            profile->ParseProperty(deviceItem->Name, deviceItem->Value);
                        }
                    }
                }
            }
        }

        // Add the new profile
        if (deviceFound)
            ProfileCache.PushBack(profile);
    }

    CacheDevice = device;
}


// Serializes the profiles to disk.
void ProfileManager::SaveCache()
{
    String path = GetProfilePath(true);
 
    Lock::Locker lockScope(&ProfileLock);

    // TODO: Since there is only a single device type now, a full tree overwrite
    // is sufficient but in the future a selective device branch replacement will
    // be necessary

    Ptr<JSON> root = *JSON::CreateObject();
    root->AddNumberItem("Oculus Profile Version", PROFILE_VERSION);
    root->AddStringItem("CurrentProfile", DefaultProfile);
    root->AddNumberItem("ProfileCount", (double) ProfileCache.GetSize());

    // Generate a JSON subtree for each profile
    for (unsigned int i=0; i<ProfileCache.GetSize(); i++)
    {
        Profile* profile = ProfileCache[i];

        JSON* json_profile = JSON::CreateObject();
        json_profile->Name = "Profile";
        json_profile->AddStringItem("Name", profile->Name);
        const char* gender;
        switch (profile->GetGender())
        {
            case Profile::Gender_Male:   gender = "Male"; break;
            case Profile::Gender_Female: gender = "Female"; break;
            default: gender = "Unspecified";
        }
        json_profile->AddStringItem("Gender", gender);
        json_profile->AddNumberItem("PlayerHeight", profile->PlayerHeight);
        json_profile->AddNumberItem("IPD", profile->IPD);

        if (profile->Type == Profile_RiftDK1)
        {
            RiftDK1Profile* riftdk1 = (RiftDK1Profile*)profile;
            JSON* json_riftdk1 = JSON::CreateObject();
            json_profile->AddItem("RiftDK1", json_riftdk1);

            const char* eyecup = "A";
            switch (riftdk1->EyeCups)
            {
                case RiftDK1Profile::EyeCup_A: eyecup = "A"; break;
                case RiftDK1Profile::EyeCup_B: eyecup = "B"; break;
                case RiftDK1Profile::EyeCup_C: eyecup = "C"; break;
            }
            json_riftdk1->AddStringItem("EyeCup", eyecup);
            json_riftdk1->AddNumberItem("LL", riftdk1->LL);
            json_riftdk1->AddNumberItem("LR", riftdk1->LR);
            json_riftdk1->AddNumberItem("RL", riftdk1->RL);
            json_riftdk1->AddNumberItem("RR", riftdk1->RR);
        }

        root->AddItem("Profile", json_profile);
    }

    root->Save(path);
}

// Returns the number of stored profiles for this device type
int ProfileManager::GetProfileCount(ProfileType device)
{
    Lock::Locker lockScope(&ProfileLock);

    if (CacheDevice == Profile_Unknown)
        LoadCache(device);

    return (int)ProfileCache.GetSize();
}

// Returns the profile name of a specific profile in the list.  The returned 
// memory is locally allocated and should not be stored or deleted.  Returns NULL
// if the index is invalid
const char* ProfileManager::GetProfileName(ProfileType device, unsigned int index)
{
    Lock::Locker lockScope(&ProfileLock);

    if (CacheDevice == Profile_Unknown)
        LoadCache(device);

    if (index < ProfileCache.GetSize())
    {
        Profile* profile = ProfileCache[index];
        OVR_strcpy(NameBuff, Profile::MaxNameLen, profile->Name);
        return NameBuff;
    }
    else
    {
        return NULL;
    }
}

bool ProfileManager::HasProfile(ProfileType device, const char* name)
{
    Lock::Locker lockScope(&ProfileLock);

    if (CacheDevice == Profile_Unknown)
        LoadCache(device);

    for (unsigned i = 0; i< ProfileCache.GetSize(); i++)
    {
        if (ProfileCache[i] && OVR_strcmp(ProfileCache[i]->Name, name) == 0)
            return true;
    }
    return false;
}


// Returns a specific profile object in the list.  The returned memory should be
// encapsulated in a Ptr<> object or released after use.  Returns NULL if the index
// is invalid
Profile* ProfileManager::LoadProfile(ProfileType device, unsigned int index)
{
    Lock::Locker lockScope(&ProfileLock);

    if (CacheDevice == Profile_Unknown)
        LoadCache(device);

    if (index < ProfileCache.GetSize())
    {
        Profile* profile = ProfileCache[index];
        return profile->Clone();
    }
    else
    {
        return NULL;
    }
}

// Returns a profile object for a particular device and user name.  The returned 
// memory should be encapsulated in a Ptr<> object or released after use.  Returns 
// NULL if the profile is not found
Profile* ProfileManager::LoadProfile(ProfileType device, const char* user)
{
    if (user == NULL)
        return NULL;

    Lock::Locker lockScope(&ProfileLock);
    
    if (CacheDevice == Profile_Unknown)
        LoadCache(device);

    for (unsigned int i=0; i<ProfileCache.GetSize(); i++)
    {
        if (OVR_strcmp(user, ProfileCache[i]->Name) == 0)
        {   // Found the requested user profile
            Profile* profile = ProfileCache[i];
            return profile->Clone();
        }
    }

    return NULL;
}

// Returns a profile with all system default values
Profile* ProfileManager::GetDeviceDefaultProfile(ProfileType device)
{
    const char* device_name = NULL;
    return CreateProfileObject("default", device, &device_name);
}

// Returns the name of the profile that is marked as the current default user.
const char* ProfileManager::GetDefaultProfileName(ProfileType device)
{
    Lock::Locker lockScope(&ProfileLock);

    if (CacheDevice == Profile_Unknown)
        LoadCache(device);

    if (ProfileCache.GetSize() > 0)
    {
        OVR_strcpy(NameBuff, Profile::MaxNameLen, DefaultProfile);
        return NameBuff;
    }
    else
    {
        return NULL;
    }
}

// Marks a particular user as the current default user.
bool ProfileManager::SetDefaultProfileName(ProfileType device, const char* name)
{
    Lock::Locker lockScope(&ProfileLock);

    if (CacheDevice == Profile_Unknown)
        LoadCache(device);
// TODO: I should verify that the user is valid
    if (ProfileCache.GetSize() > 0)
    {
        DefaultProfile = name;
        Changed = true;
        return true;
    }
    else
    {
        return false;
    }
}


// Saves a new or existing profile.  Returns true on success or false on an
// invalid or failed save.
bool ProfileManager::Save(const Profile* profile)
{
    Lock::Locker lockScope(&ProfileLock);

    if (OVR_strcmp(profile->Name, "default") == 0)
        return false;  // don't save a default profile

    // TODO: I should also verify that this profile type matches the current cache
    if (CacheDevice == Profile_Unknown)
        LoadCache(profile->Type);

    // Look for the pre-existence of this profile
    bool added = false;
    for (unsigned int i=0; i<ProfileCache.GetSize(); i++)
    {
        int compare = OVR_strcmp(profile->Name, ProfileCache[i]->Name);
              
        if (compare == 0)
        {   
            // TODO: I should do a proper field comparison to avoid unnecessary
            // overwrites and file saves

            // Replace the previous instance with the new profile
            ProfileCache[i] = *profile->Clone();
            added   = true;
            Changed = true;
            break;
        }
    }

    if (!added)
    {
        ProfileCache.PushBack(*profile->Clone());
        if (ProfileCache.GetSize() == 1)
            CacheDevice = profile->Type;

        Changed = true;
    }

    return true;
}

// Removes an existing profile.  Returns true if the profile was found and deleted
// and returns false otherwise.
bool ProfileManager::Delete(const Profile* profile)
{
    Lock::Locker lockScope(&ProfileLock);

    if (OVR_strcmp(profile->Name, "default") == 0)
        return false;  // don't delete a default profile

    if (CacheDevice == Profile_Unknown)
        LoadCache(profile->Type);

    // Look for the existence of this profile
    for (unsigned int i=0; i<ProfileCache.GetSize(); i++)
    {
        if (OVR_strcmp(profile->Name, ProfileCache[i]->Name) == 0)
        {  
            if (OVR_strcmp(profile->Name, DefaultProfile) == 0)
                DefaultProfile.Clear();
            
            ProfileCache.RemoveAt(i);
            Changed = true;
            return true;
        }
    }

    return false;
}



//-----------------------------------------------------------------------------
// ***** Profile

Profile::Profile(ProfileType device, const char* name)
{
    Type         = device;
    Gender       = Gender_Unspecified;
    PlayerHeight = 1.778f;    // 5'10" inch man
    IPD          = 0.064f;
    OVR_strcpy(Name, MaxNameLen, name);
}


bool Profile::ParseProperty(const char* prop, const char* sval)
{
    if (OVR_strcmp(prop, "Name") == 0)
    {
        OVR_strcpy(Name, MaxNameLen, sval);
        return true;
    }
    else if (OVR_strcmp(prop, "Gender") == 0)
    {
        if (OVR_strcmp(sval, "Male") == 0)
            Gender = Gender_Male;
        else if (OVR_strcmp(sval, "Female") == 0)
            Gender = Gender_Female;
        else
            Gender = Gender_Unspecified;

        return true;
    }
    else if (OVR_strcmp(prop, "PlayerHeight") == 0)
    {
        PlayerHeight = (float)atof(sval);
        return true;
    }
    else if (OVR_strcmp(prop, "IPD") == 0)
    {
        IPD = (float)atof(sval);
        return true;
    }

    return false;
}


// Computes the eye height from the metric head height
float Profile::GetEyeHeight()
{
    const float EYE_TO_HEADTOP_RATIO =   0.44538f;
    const float MALE_AVG_HEAD_HEIGHT =   0.232f;
    const float FEMALE_AVG_HEAD_HEIGHT = 0.218f;
     
    // compute distance from top of skull to the eye
    float head_height;
    if (Gender == Gender_Female)
        head_height = FEMALE_AVG_HEAD_HEIGHT;
    else
        head_height = MALE_AVG_HEAD_HEIGHT;

    float skull = EYE_TO_HEADTOP_RATIO * head_height;

    float eye_height  = PlayerHeight - skull;
    return eye_height;
}


//-----------------------------------------------------------------------------
// ***** RiftDK1Profile

RiftDK1Profile::RiftDK1Profile(const char* name) : Profile(Profile_RiftDK1, name)
{
    EyeCups = EyeCup_A;
    LL = 0;
    LR = 0;
    RL = 0;
    RR = 0;
}

bool RiftDK1Profile::ParseProperty(const char* prop, const char* sval)
{
    if (OVR_strcmp(prop, "EyeCup") == 0)
    {
        switch (sval[0])
        {
            case 'C': EyeCups = EyeCup_C; break;
            case 'B': EyeCups = EyeCup_B; break;
            default:  EyeCups = EyeCup_A; break;
        }
        return true;
    }
    else if (OVR_strcmp(prop, "LL") == 0)
    {
        LL = atoi(sval);
        return true;
    }
    else if (OVR_strcmp(prop, "LR") == 0)
    {
        LR = atoi(sval);
        return true;
    }
    else if (OVR_strcmp(prop, "RL") == 0)
    {
        RL = atoi(sval);
        return true;
    }
    else if (OVR_strcmp(prop, "RR") == 0)
    {
        RR = atoi(sval);
        return true;
    }
    
    return Profile::ParseProperty(prop, sval);
}

Profile* RiftDK1Profile::Clone() const
{
    RiftDK1Profile* profile = new RiftDK1Profile(*this);
    return profile;
}

}  // OVR
