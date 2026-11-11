#pragma once
#include "DayNight.h"
#include "Hud.h"

struct GLFWwindow;

// Keyboard/mouse controls for the time of day and the on-screen help (HUD).
//
//   Space        pause / resume time
//   - / +        slower / faster
//   Left/Right   scrub time backwards / forwards (hold)
//   1 2 3 4      jump to morning / noon / sunset / night
//   T            time-lapse from morning to night
//   H            show / hide help
//   Left mouse   drag the time slider at the bottom of the screen
class TimeControls {
public:
    explicit TimeControls(DayNight& dayNight);

    void onKey(int key, int action);
    // returns true if the click hit the time slider
    bool onMouseButton(int button, int action, double x, double y, int windowWidth, int windowHeight);
    void onMouseMove(double x, double y, int windowWidth, int windowHeight);

    // held keys and the end of the time-lapse
    void update(GLFWwindow* window, float deltaTime);

    // true once after the hour was set directly (jump, slider, time-lapse start),
    // so main can put the lamp into the right state for the new hour
    bool takeJump();

    void draw(Hud& hud, int windowWidth, int windowHeight, float fps, bool lampOn) const;

private:
    struct Box { float x, y, w, h; };
    static Box sliderBox(int windowWidth, int windowHeight);
    void setHourFromMouse(double x, int windowWidth, int windowHeight);
    void jumpTo(float hour);
    void changeSpeed(int step);

    DayNight& dayNight;
    int speedLevel;
    bool showHelp = true;
    bool draggingSlider = false;
    bool timeLapse = false;
    bool jumped = false;
};
