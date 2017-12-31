/*
 * angles.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <logging.hpp>
#include "common.hpp"

namespace angles
{

struct angle_data_s
{
    static constexpr size_t count = 16;
    inline void push(const Vector &angle)
    {
        if (not angle.x and not angle.y)
            return;
        good                = true;
        angles[angle_index] = angle;
        if (++angle_index >= count)
        {
            angle_index = 0;
        }
        /*if (angle_count > 0) {
            int ai = angle_index - 2;
            if (ai < 0) ai = count - 1;
            float dx = std::abs(angles[ai].x - angle.x);
            float dy = std::abs(angles[ai].y - angle.y);
            if (sqrt(dx * dx + dy * dy) > 45.0f) {
                //logging::Info("%.2f %.2f %.2f", dx, dy, sqrt(dx * dx + dy *
        dy));
            }
        }*/
        if (angle_count < count)
        {
            angle_count++;
        }
    }
    inline float deviation(int steps) const
    {
        int j    = angle_index - 2;
        int k    = j + 1;
        float hx = 0, hy = 0;
        for (int i = 0; i < steps && i < angle_count; i++)
        {
            if (j < 0)
                j = count + j;
            if (k < 0)
                k = count + k;

            float dev_x = std::abs(angles[k].x - angles[j].x);
            float dev_y = std::abs(angles[k].y - angles[j].y);
            if (dev_x > hx)
                hx = dev_x;
            if (dev_y > hy)
                hy = dev_y;

            // logging::Info("1: %.2f %.2f | 2: %.2f %.2f | dev: %.2f",
            // angles[k].x, angles[k].y, angles[j].x, angles[j].y, sqrt(dev_x *
            // dev_x + dev_y * dev_y));

            --j;
            --k;
        }
        if (hy > 180)
            hy = 360 - hy;
        return sqrt(hx * hx + hy * hy);
    }

    Vector angles[count]{};
    bool good{ false };
    int angle_index{ 0 };
    int angle_count{ 0 };
};

extern angle_data_s data_[32];

void Update();

inline angle_data_s &data_idx(int index)
{
    if (index < 1 || index > 32)
    {
        throw std::out_of_range("Angle table entity index out of range");
    }
    return data_[index - 1];
}

inline angle_data_s &data(const CachedEntity *entity)
{
    return data_idx(entity->m_IDX);
}
}
