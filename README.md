# Simple Gamepad Mouse

Maps a gamepad to the mouse. Also makes it possible to type using morse code (yes it's fast).

## Buttons

| Key | Action                | Modifier |
| :-: | ---                   | :-:      |
| A   | left-click            | -        |
| B   | right-click           | -        |
| X   | backspace             | -        |
| Y   | space                 | -        |
| LS  | escape                | -        |
| RS  | middle-click          | -        |
| A   | morse dot             | RB       |
| B   | morse dash            | RB       |
| Y   | word/space            | RB       |
| X   | morse reset/backspace | RB       |
| Y   | play/pause            | LB       |

## Axes

| Axis | Action      | Modifier |
| :-:  | ---         | :-:      |
| LS   | mouse       | -        |
| RS   | scroll      | -        |
| LS   | slow mouse  | LB       |
| RS   | slow scroll | LB       |

## Morse Code

- When RB is held, the morse code tree is shown and A and B are mapped to short and long, respectively.
- Typing a faulty morse code, or pressing X, will reset the current morse code sequence without writing any character.
- Idling for 500ms after a sequence has been entered, or pressing Y, will write the sequence character.
- Pressing Y when no sequence has been entered will write a space character.