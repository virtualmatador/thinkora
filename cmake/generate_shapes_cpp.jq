def filename: split("/")[-1];
def stem: filename | sub("\\.json$"; "");
def shape_name: stem | split("_")[0];
def n: tostring;
def angle: (n + " * std::numbers::pi / 180.0");
def convex_initializer:
  .frame as $frame
  | .convex as $convex
  | [
      "            Convex{ConvexData{",
      "                .frame = {{{" + ($frame.left | n) + ", " + ($frame.top | n) + "}, {" + ($frame.right | n) + ", " + ($frame.bottom | n) + "}}},",
      "                .b_a = " + (($convex.b_a // 0.0) | angle) + ",",
      "                .b_x = " + (($convex.b_x // 0.0) | n) + ",",
      "                .b_y = " + (($convex.b_y // 0.0) | n) + ",",
      "                .e_a = " + (($convex.e_a // 0.0) | angle) + ",",
      "                .e_x = " + (($convex.e_x // 0.0) | n) + ",",
      "                .e_y = " + (($convex.e_y // 0.0) | n) + ",",
      "                .d_a = " + (($convex.d_a // 0.0) | angle) + ",",
      "                .n_b = " + ($convex.n_b | n) + ",",
      "                .n_e = " + ($convex.n_e | n) + ",",
      "            }},"
    ] | join("\n");
def shape_initializer:
  .file as $file
  | .data as $data
  | [
      "    Character{",
      "        \"" + ($file | filename | stem | shape_name) + "\",",
      "        {",
      ($data[] | convex_initializer),
      "        },",
      "    },"
    ] | join("\n");

[inputs as $data | {file: input_filename, data: $data}] as $sources
| [
    "#include \"shapes.h\"",
    "",
    "#include <array>",
    "#include <numbers>",
    "",
    "namespace",
    "{",
    "const std::array<Character, " + ($sources | length | tostring) + "> shapes {{",
    ($sources[] | shape_initializer),
    "}};",
    "}",
    "",
    "std::span<const Character> get_shapes()",
    "{",
    "    return shapes;",
    "}"
  ] | join("\n")
