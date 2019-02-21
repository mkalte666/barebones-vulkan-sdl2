/*
    triangle.cpp: A Hello Triangle example
    Copyright (C) 2019 Malte Kieﬂling

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <SDL.h>

#include <exception>

#include "application.h"

int main(int argc, char** argv)
{
    try {
        ApplicationCreateInfo info;
        info.title = "Hello Triangle";
        Application app(info);
        app.run();
    } catch (std::exception err) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", err.what(), nullptr);
        exit(-1);
    }
    catch (...)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", "Something unknown went terribly wrong!", nullptr);
        exit(-1);
    }

    return 0;
}