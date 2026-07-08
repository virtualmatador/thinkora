Shape patterns use the same convex JSON format as character patterns.

The initial set is intentionally small and focused on common drawing targets:
line, arrow, circle, square, rectangle, triangle, and diamond.

Pattern files are recognition variants. Their file names point to render files:

```
shapes/database_01.json -> render/database.json
shapes/database_02.json -> render/database.json
```

The shape name is the file stem with the final `_NN` variant suffix removed.
These files are data only for now. The OCR pipeline should later load them
separately from characters, then draw the matched shape from the normalized
geometry in `render/` instead of converting it to text.
