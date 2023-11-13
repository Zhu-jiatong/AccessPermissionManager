#if !defined(_AccessPermissionManager_h)
#define _AccessPermissionManager_h

#include <Arduino.h>
#include <vector>
#include <bitset>
#include <DatabaseConnection.h>

using FilePermissions = std::bitset<2>;

enum Permission
{
	READ = 1,
	WRITE
};

class AccessPermissionManager
{
public:
	void open(const String &sharingRegistryPath);
	bool canUserAccessFile(const String &username, const String &path, FilePermissions permissions);
	void shareFileWithUser(const String &path, const String &username, FilePermissions permissions);
	void unshareFileWithUser(const String &path, const String &username);
	void unshareFile(const String &path);
	std::vector<String> getUsersWithAccessToFile(const String &path, FilePermissions permissions);
	std::vector<String> getFilesSharedWithUser(const String &username, FilePermissions permissions);

private:
	DatabaseConnection _sharingRegistry;

	String generateSharingId(const String &user, const String &path);
};

#endif // _AccessPermissionManager_h
