
#include "common.hpp"
#include "drawing.hpp"
#include "Menu.hpp"
#include "GuiInterface.hpp"

namespace snowflakes
{

// Control variables
// Base Speed the flakes spawn at
static settings::Float min_snowflake_speed_x{ "visual.snowflakes.min-speed.x", "0.0f" };
static settings::Float max_snowflake_speed_x{ "visual.snowflakes.max-speed.x", "00.0f" };
static settings::Float min_snowflake_speed_y{ "visual.snowflakes.min-speed.y", "10.0f" };
static settings::Float max_snowflake_speed_y{ "visual.snowflakes.max-speed.y", "150.0f" };

// Speed Variance
static settings::Float snowflake_speed_deviation_x{ "visual.snowflakes.speed-variance.x", "5.0f" };
static settings::Float snowflake_speed_deviation_y{ "visual.snowflakes.speed-variance.y", "30.0f" };

// Snowflake size
static settings::Float snowflake_size{ "visual.snowflakes.size", "16.0f" };

// Snowflake Amount
static settings::Int snowflake_amount{ "visual.snowflakes.count", "150" };

// Snowflake class
class Snowflake
{
    // Position
    float x{};
    float y{};
    // Fall speed (pixel/s)
    float x_speed{};
    float y_speed{};
    // Fall speed variation (in pixels)
    float x_speed_variation{};
    float y_speed_variation{};
    // Neccessary to make snowflakes not depend on frames
    float last_update_time{};
    // Makes Snowflakes chaotic because they all run on their own timer
    std::chrono::milliseconds start_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

public:
    // Needed for std::find
    constexpr bool operator==(const Snowflake &rhs) const
    {
        // Ignore the warnings, this is perfectly valid syntax in this case
        return x == rhs.x && y == rhs.y && x_speed == rhs.x_speed && x_speed_variation == rhs.x_speed_variation && y_speed == rhs.y_speed && y_speed_variation == rhs.y_speed_variation;
    }

    // Init the snowflake
    void Init(float x, float y, float x_speed, float y_speed, float x_speed_variation = 0.0f, float y_speed_variation = 0.0f)
    {
        // Starting point
        this->x = x;
        this->y = y;
        // Base fall speed (pixel/s)
        this->x_speed = x_speed;
        this->y_speed = y_speed;
        // Variation amount (pixel/s)
        this->x_speed_variation = x_speed_variation;
        this->y_speed_variation = y_speed_variation;

        // Update time
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - start_ms;
        auto cur_time                = ms.count() / 1000.0f;
        this->last_update_time       = cur_time;
    }
    // Update x and y coords
    void Update(std::chrono::milliseconds current_time)
    {
        // Every snowflake has it's own one of these, so use it.
        current_time -= start_ms;
        auto cur_time = current_time.count() / 1000.0f;
        // Ignore update after 5 seconds timeout
        if (cur_time - this->last_update_time <= 5.0f)
        {
            float variation_x, variation_y;

            // Update speed

            // We want a smooth variation, so let's use sine/cosine
            variation_x = (sin(DEG2RAD(cur_time * 180.0f)) * x_speed_variation);
            // The +1 and /2.0f is so snowflakes don't just freeze and stop moving
            variation_y = (cos(DEG2RAD(cur_time * 180.0f) + 1) * y_speed_variation / 2.0f);

            // Move this speed in pixels/s
            this->x += (x_speed + variation_x) * (cur_time - this->last_update_time);
            this->y += (y_speed + variation_y) * (cur_time - this->last_update_time);

            // Update last draw time accordingly
        }
        this->last_update_time = cur_time;
    }
    // Draw the Snowflake
    void Draw() const
    {
        draw::RectangleTextured(this->x, this->y, *snowflake_size, *snowflake_size, colors::white, textures::atlas().texture, 257, 0, 16, 16, 0.0f);
    }
    // Is the Snowflake off-screen?
    bool IsOffScreen() const
    {
        // We don't care too much about snowflakes going off of the left/right of the screen
        return y - *snowflake_size > draw::height || y < -10.0f;
    }
};

// Snowflake Manager
class SnowflakeManager
{
    std::vector<Snowflake> snowflakes;

    // Remove a(n off-screen) snowflake
    void RemoveSnowflake(Snowflake &flake)
    {
        auto pos = std::find(snowflakes.begin(), snowflakes.end(), flake);
        if (pos != snowflakes.end())
            snowflakes.erase(pos);
    }

public:
    // Add a Snowflake to the vector
    void AddSnowflake()
    {
        snowflakes.resize(snowflakes.size() + 1);
        Snowflake &flake = snowflakes[snowflakes.size() - 1];

        // Random position at top of screen
        float flake_x = RandFloatRange(0.0f, draw::width);
        float flake_y = 0.0f;

        // Random Speed
        float flake_speed_x = RandFloatRange(*min_snowflake_speed_x, *max_snowflake_speed_x);
        float flake_speed_y = RandFloatRange(*min_snowflake_speed_y, *max_snowflake_speed_y);

        // Init flake
        flake.Init(flake_x, flake_y, flake_speed_x, flake_speed_y, *snowflake_speed_deviation_x, *snowflake_speed_deviation_y);
    }

    // Update and Draw the snowflakes
    void UpdateAndDraw()
    {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

        // Update loop
        for (auto &flake : snowflakes)
        {
            flake.Update(ms);
            // Remove offscreen flakes
            if (flake.IsOffScreen())
                RemoveSnowflake(flake);
            else
                flake.Draw();
        }
        if (snowflakes.size() < static_cast<unsigned>(*snowflake_amount))
            AddSnowflake();
    }
    // Clear all snowflakes
    void Clear()
    {
        snowflakes.clear();
    }
};

static SnowflakeManager snow_manager;
void DrawSnowflakes()
{
    if (zerokernel::Menu::instance && !zerokernel::Menu::instance->isInGame())
        snow_manager.UpdateAndDraw();
}

template <typename T> void rvarCallback(settings::VariableBase<T> &, T)
{
    snow_manager.Clear();
}

static InitRoutine init(
    []()
    {
        std::time_t theTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm *aTime      = std::localtime(&theTime);

        int day   = aTime->tm_mday;
        int month = aTime->tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12 concept

        // We only want to draw around christmas time, let's use 12th of december+ til 12th of january
        if ((month == 12 && day >= 12) || (month == 1 && day <= 12))
        {
            // Very early to not draw over the menu
            EC::Register(EC::Draw, DrawSnowflakes, "draw_snowflakes", EC::ec_priority::very_early);
            min_snowflake_speed_x.installChangeCallback(rvarCallback<float>);
            max_snowflake_speed_x.installChangeCallback(rvarCallback<float>);
            min_snowflake_speed_y.installChangeCallback(rvarCallback<float>);
            max_snowflake_speed_y.installChangeCallback(rvarCallback<float>);
            snowflake_speed_deviation_x.installChangeCallback(rvarCallback<float>);
            snowflake_speed_deviation_y.installChangeCallback(rvarCallback<float>);
        }
    });
} // namespace snowflakes
