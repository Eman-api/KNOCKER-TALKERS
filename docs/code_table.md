# 5-bit code table

Left flex = `0`, right flex = `1`. After the **5th** flex the letter is sent automatically.
Made a mistake mid-letter? Flex **both** wrists to erase the bits and start the letter over.

**Rule of thumb:** a letter's code is its position in the alphabet in binary (A = 1 = `00001`, Z = 26 = `11010`).

| Letter | Code | Letter | Code |
|---|---|---|---|
| A | `00001` | B | `00010` |
| C | `00011` | D | `00100` |
| E | `00101` | F | `00110` |
| G | `00111` | H | `01000` |
| I | `01001` | J | `01010` |
| K | `01011` | L | `01100` |
| M | `01101` | N | `01110` |
| O | `01111` | P | `10000` |
| Q | `10001` | R | `10010` |
| S | `10011` | T | `10100` |
| U | `10101` | V | `10110` |
| W | `10111` | X | `11000` |
| Y | `11001` | Z | `11010` |

| Command | Code |
|---|---|
| space | `11011` |
| . (speak sentence) | `11100` |
| ? (speak question) | `11101` |
| DELETE last letter | `11110` |
| HELP (call for help) | `11111` |
| CANCEL false alarm | `00000` |

Example: **HI.** = `01000` `01001` `11100` → 15 flexes (7-bit ASCII would need 21 flexes + 3 commits = 24).
