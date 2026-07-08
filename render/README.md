Render files describe normalized output geometry for recognized shapes.

A shape recognition pattern named `shapes/<name>_NN.json` maps to one
single-render file named `render/<name>.json`. The render name is the file stem;
there is no `name` field in the JSON.

Each render file has exactly one canonical rendering. Logical drawing variants
are represented by recognition files in `shapes/`:

```
shapes/database_01.json -> render/database.json
shapes/database_02.json -> render/database.json
```

The OCR result should strip the final `_NN` suffix from the matched shape
pattern name, load the corresponding render file, then transform its normalized
geometry using the matched drawing frame, cleaned rotation, size, and aspect
ratio.

Current schema:

```
[
    {
        "type": "polyline",
        "closed": false,
        "points": [[0.1, 0.5], [0.9, 0.5]]
    },
    {
        "type": "arc",
        "center": [0.5, 0.5],
        "radius": 0.45,
        "start_angle": 0.0,
        "end_angle": 360.0
    }
]
```

Coordinates and radii are normalized to the 0..1 render box. Angles are degrees.
Arc path items represent perfect curves and can use any start/end angle.
