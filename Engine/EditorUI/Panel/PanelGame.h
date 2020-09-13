#ifndef _PANELGAME_H_
#define _PANELGAME_H_

#include "Panel.h"
#include <MathGeoLib.h>

class Viewport;

class PanelGame : public Panel
{
public:
	PanelGame();
	~PanelGame();

	void Render() override;

private:
	void ShowEmptyGameWindow() const;

public:
	float game_window_content_area_width = 0;
	float game_window_content_area_height = 0;
	float2 game_window_content_area_pos = float2::zero;

private:
	Viewport* game_viewport = nullptr;
};

#endif //_PANELGAME_H_
