// Copyright (c) 2024, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once

#include <list>
#include <string>
#include <vector>

namespace alib {
class Stream;
}


namespace PuyoVS::ClientNG {
class PlaylistEntry {
public:
	PlaylistEntry();
	PlaylistEntry(std::string dir, std::string entry );

	bool isValid() const;
	std::string toString();

	std::string url;
	unsigned urlHash;
	std::vector<std::string> params;

};

class Playlist {
public:
	Playlist();
	~Playlist();

	void insert(PlaylistEntry entry, int id);
	void add(PlaylistEntry entry);
	void remove(int id);
	PlaylistEntry take(int id);
	void move(int count, int id_from, int id_to);

private:
	std::list<PlaylistEntry> m_playlist;
	const char* m_playlistFn, m_discoverPath;
};

class MusicPlayer {
public:

};

}
