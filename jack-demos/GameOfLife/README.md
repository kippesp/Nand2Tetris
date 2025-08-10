Resources:
- https://classes.engineering.wustl.edu/cse365/jack.php

Game of life

Premise:
- cell: dead | alive
- cell has 8 neighbors
- each generation change happens according to rules

Rules:
- given cell == alive
    - if less then 2 neighbors are alive - die (underpopulation)
    - if 2, 3 neighbors are alive -> live
    - if more then 3 neighbors are alive -> die (overpopulation)
- given cell == dead
    - if 3 live neighbors -> live


