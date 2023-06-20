# digital-scoreboard
The code related to a digital scoreboard project.

Will update as progress is made...


## Overview

This project was brought about for a need to be able to keep score for volleyball games. Aside from keeping the score
of two teams and the game number, it must be able to be updated remotely. The idea is have a small, but durable, remote
that can be kept in the pocket or mounted to the net or pole without any adverse effects.

The goal is to create everything for the scoreboard from scratch as much as possible. Parts will be 3D print if possible.

## Code

There are currenly two main components in this project. The first component being the scoreboard itself. This will
house all the components and where the score will be displayed. The second component being the bluetooth remote
control.The remote will be small and contain only buttons to change the score for either team. Each component has
separate code to be loaded.

* Main board - `scoreboard-main.ino`
* Bluetooth remote - `scoreboard-bluetooth-remote.ino`

