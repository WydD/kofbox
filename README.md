# kofbox

Display hitboxes inside King of Fighter XIV.

![](result.png?raw=true)

## How to?

* Go to the [release page](https://github.com/WydD/kofbox/releases), and get the latest release.
* Extract the zip archive somewhere
* Launch your game
* Launch the provided exe

It should display something like "KOFBox is injected, everything is alright.", type enter to close the window. 
You should see boxes now!

## Where does it work?
Basically everywhere. Even online if you are willing to play with this on.

## Hide the boxes
To hide the boxes after the injection. Press F5. To re-enable them, repress F5.

## Will this hold against updates?
It held during all beta updates so, "most surely".

## How do you do that?
Well the code is available but I use plenty of dark magic there so... can't tell you much more.

That said, I use easyhook to inject a dll inside the game, then it's _just_ a matter of reading the memory and drawing 
rectangles.

## Hey you've got frame count inside your inputs!
This project embeds https://github.com/WydD/kof-xiv-input-frames which enables frame counts in input display. 

## Is it safe?
I think so, nothing prevents code injection in the game so we're good.

No gameplay element is modified, so technically you could even play online with it.

## Building this
You've got a CMake you should be able to do this using VS compiler.