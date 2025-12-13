#pragma once
#include <string>
#include <stdexcept>
#include <limits> 
#include "../controller/MusicPlayer.hpp"
#include "PlaybackQueue.hpp"
#include "PlaybackHistory.hpp"
#include "PlayNextQueue.hpp"


namespace models{
	void addAlbumToQueue(const std::string& albumName, const models::MusicLibrary& library, models::PlaybackQueue& queue);
}