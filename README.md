# c-astro

**c-astro** is a small library and collection of tools for image processing,
written primarily for my own use in amateur astrophotography, although you may
find other uses for them.

## status

wip

## bugs

the atronometric functions are not terribly accurate.  among other things,
the equatorial coordinates used are not based on ICRS, and the equation of
equinox is not based on any standard or IAU resolution, but rather put together
from various formulae found here and there.

the camera model is a simple pin-hole.

things may break if you play with files across architectures with mixed
endianness
