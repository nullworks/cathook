/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/object/input/Slider.hpp>
#include <menu/Menu.hpp>

namespace zerokernel
{
settings::RVariable<int> SliderStyle::default_width{ "zk.style.input.slider.width", "60" };
settings::RVariable<int> SliderStyle::default_height{ "zk.style.input.slider.height", "14" };

settings::RVariable<int> SliderStyle::handle_width{ "zk.style.input.slider.handle_width", "6" };
settings::RVariable<int> SliderStyle::bar_width{ "zk.style.input.slider.bar_width", "2" };
} // namespace zerokernel
