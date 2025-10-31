rule json_starts_utf8_better {
  meta:
    category = "json"
    confidence = "high"

  strings:
    // Start: optional whitespace, then object:
    // either '}' (empty) or a "key":
    $obj_start = /^\s*\{\s*(\}|"(\\.|[^"\\])+"\s*:)/

    // Start: optional whitespace, then array:
    // either ']' (empty) or first value: "…", number, true/false/null, {, [
    $arr_start = /^\s*\[\s*(\]|"(\\.|[^"\\])+"|[-]?\d|true|false|null|\{|\[)/i

  condition:
    $obj_start or $arr_start
}

