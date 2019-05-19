# Painter Overhaul for Hellshots Golf

This is an addon for Hellshots Golf which adds two new weapons, NewPainter and NewPaintGun.
These are recoded versions of the Painter and PaintWand in Hellshots Golf, and have
the following improvements:

- When holding the Reload button, your aim is locked in place, and a cursor appears on
  your screen. When painting, your stream of paint will follow the cursor. This works
  with the Software renderer, Zandronum's OpenGL renderer, GZDoom's softpoly renderer,
  and GZDoom's OpenGL renderer.

- The color selection menu has been redone. Now you can select colors by moving your
  mouse towards the color in question, as well as using the movement keys as before.
  Your selection isn't forgotten when you let go of the movement keys or stop moving
  the mouse (unlike the old system), and when selecting a diagonal color with the
  movement keys, you're much less likely to accidentally select one of the nearby
  colors when letting go of the keys. Also, if you were holding any movement keys
  when entering the menu, those are ignored until you depress or release a key,
  allowing you to painlessly select a color with the mouse when moving from place
  to place.

- The paint cursor and color selection menu have been clientsided, which means your
  ping will not affect the responiveness of either of them.

- Painting is now continuous, no matter how fast you move the mouse. This is limited
  online, for bandwidth purposes.

- The painters have been made more responsive in general.


Truthfully, I made this to put my screen-to-world ACS to use, and because the color
menu annoyed me. The only other screen-to-world code I'm aware of for Doom is coded
in ZScript, which is unavailable to Zandronum.