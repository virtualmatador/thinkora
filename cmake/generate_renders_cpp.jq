def filename: split("/")[-1];
def stem: filename | sub("\\.json$"; "");
def ident:
  gsub("[^A-Za-z0-9_]"; "_")
  | if test("^[0-9]") then "_" + . else . end;
def n: tostring;
def angle: (n + " * std::numbers::pi / 180.0");
def bool: if . then "true" else "false" end;
def point_initializer: "{" + (.[0] | n) + ", " + (.[1] | n) + "}";
def polyline_points($name; $index; $path):
  [
    "const std::array<Point, " + ($path.points | length | tostring) + "> render_" + $name + "_" + ($index | tostring) + "_points {{",
    ($path.points[] | "    " + point_initializer + ","),
    "}};"
  ] | join("\n");
def path_initializer($name; $index):
  if .type == "polyline" then
    "    RenderPath{ .type = RenderPath::Type::Polyline, .points = render_" + $name + "_" + ($index | tostring) + "_points, .closed = " + (.closed | bool) + " },"
  elif .type == "arc" then
    "    RenderPath{ .type = RenderPath::Type::Arc, .points = {}, .closed = false, .center = " + (.center | point_initializer) + ", .radius = " + (.radius | n) + ", .start_angle = " + (.start_angle | angle) + ", .end_angle = " + (.end_angle | angle) + " },"
  else
    error("unknown render path type: " + .type)
  end;
def render_arrays:
  .file as $file
  | .data as $data
  | ($file | filename | stem | ident) as $id
  | [
      ($data | to_entries[] | select(.value.type == "polyline") | polyline_points($id; .key; .value)),
      "const std::array<RenderPath, " + ($data | length | tostring) + "> render_" + $id + "_paths {{",
      ($data | to_entries[] | . as $entry | ($entry.value | path_initializer($id; $entry.key))),
      "}};"
    ] | join("\n");
def render_source:
  .file as $file
  | ($file | filename | stem) as $name
  | ($name | ident) as $id
  | "    { .name = \"" + $name + "\", .paths = render_" + $id + "_paths },";

[inputs as $data | {file: input_filename, data: $data}] as $sources
| [
    "#include \"renders.h\"",
    "",
    "#include <array>",
    "#include <numbers>",
    "",
    "namespace",
    "{",
    ($sources[] | render_arrays),
    "const std::array<RenderSource, " + ($sources | length | tostring) + "> renders {{",
    ($sources[] | render_source),
    "}};",
    "}",
    "",
    "std::span<const RenderSource> get_renders()",
    "{",
    "    return renders;",
    "}"
  ] | join("\n")
