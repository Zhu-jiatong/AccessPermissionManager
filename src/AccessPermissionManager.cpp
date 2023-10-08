#include "AccessPermissionManager.h"
#include <mbedtls/sha256.h>
#include <sstream>
#include <iomanip>
#include "FilesystemUtility/src/FilesystemUtility.h"

void AccessPermissionManager::open(const String &sharingRegistryPath)
{
	_sharingRegistry.open(sharingRegistryPath);
	_sharingRegistry.execute("CREATE TABLE IF NOT EXISTS users_registry (username TEXT NOT NULL, sharing_id TEXT NOT NULL)");
	_sharingRegistry.execute("CREATE TABLE IF NOT EXISTS files_registry (sharing_id TEXT NOT NULL PRIMARY KEY, path TEXT NOT NULL UNIQUE) WITHOUT ROWID");
}

bool AccessPermissionManager::canUserAccessFile(const String &username, const String &path)
{
	if (FSUtil::isChildOf("/" + username, path))
		return true;

	JSONVar sharingIdResult = _sharingRegistry.execute("SELECT sharing_id FROM files_registry WHERE path=?", path);
	if (sharingIdResult.length() == -1)
		return false;

	String sharingId = sharingIdResult[0]["sharing_id"];

	JSONVar result = _sharingRegistry.execute("SELECT * FROM users_registry WHERE username=? AND sharing_id=?", username, sharingId);
	if (result.length() == -1)
		return false;
	return true;
}

void AccessPermissionManager::shareFileWithUser(const String &path, const String &username)
{
	JSONVar sharingIdResult = _sharingRegistry.execute("SELECT sharing_id FROM files_registry WHERE path=?", path);
	if (sharingIdResult.length() == -1)
	{
		String sharingId = generateSharingId(username, path);
		_sharingRegistry.execute("INSERT INTO files_registry VALUES (?, ?)", sharingId, path);
		_sharingRegistry.execute("INSERT INTO users_registry VALUES (?, ?)", username, sharingId);
	}
	else
	{
		String sharingId = sharingIdResult[0]["sharing_id"];
		_sharingRegistry.execute("INSERT INTO users_registry VALUES (?, ?)", username, sharingId);
	}
}

void AccessPermissionManager::unshareFileWithUser(const String &path, const String &username)
{
	JSONVar sharingIdResult = _sharingRegistry.execute("SELECT sharing_id FROM files_registry WHERE path=?", path);

	if (sharingIdResult.length() == -1)
		throw std::runtime_error("file not shared with user");

	String sharingId = sharingIdResult[0]["sharing_id"];
	_sharingRegistry.execute("DELETE FROM users_registry WHERE username=? AND sharing_id=?", username, sharingId);
}

void AccessPermissionManager::unshareFile(const String &path)
{
	JSONVar sharingIdResult = _sharingRegistry.execute("SELECT sharing_id FROM files_registry WHERE path=?", path);
	if (sharingIdResult.length() == -1)
		throw std::runtime_error("file not shared");

	String sharingId = sharingIdResult[0]["sharing_id"];
	_sharingRegistry.execute("DELETE FROM users_registry WHERE sharing_id=?", sharingId);
	_sharingRegistry.execute("DELETE FROM files_registry WHERE path=?", path);
}

std::vector<String> AccessPermissionManager::getUsersWithAccessToFile(const String &path)
{
	JSONVar sharingIdResult = _sharingRegistry.execute("SELECT sharing_id FROM files_registry WHERE path=?", path);
	if (sharingIdResult.length() == -1)
		return std::vector<String>();

	String sharingId = sharingIdResult[0]["sharing_id"];
	JSONVar result = _sharingRegistry.execute("SELECT username FROM users_registry WHERE sharing_id=?", sharingId);
	if (result.length() == -1)
		return std::vector<String>();

	std::vector<String> users;
	for (int i = 0; i < result.length(); i++)
		users.push_back(result[i]["username"]);
	return users;
}

String AccessPermissionManager::generateSharingId(const String &user, const String &path)
{
	uint8_t sharingId[32];
	String input = user + path + esp_random();

	int result = mbedtls_sha256_ret(
		reinterpret_cast<const uint8_t *>(input.c_str()),
		input.length(),
		sharingId,
		0);

	if (result)
		throw std::runtime_error("cannot generate sharing ID");

	std::stringstream resultStringBuilder;
	for (auto &&i : sharingId)
		resultStringBuilder << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);

	return resultStringBuilder.str().c_str();
}