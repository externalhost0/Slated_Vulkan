import re

# Simulated input: C++ file content with annotated comments above sol::new_usertype
cpp_code = """
// @lua Vec2
// @field x number
// @field y number
// @constructor Vec2() Vec2(float) Vec2(float, float)
// @operator + - * / ==
// @description 2D vector class with basic operations
_solstate.new_usertype<glm::vec2>(
    "Vec2",
    sol::call_constructor, sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float)>(),
    "x", sol::property(&glm::vec2::x),
    "y", sol::property(&glm::vec2::y)
);
"""

# Parse the annotated comment block
def parse_lua_bindings(cpp_code):
    pattern = re.compile(r"""
        //\s@lua\s(?P<class>\w+)\n
        (//\s@field\s(?P<fields>.*?)\n)?
        (//\s@field\s(?P<morefields>.*?)\n)?
        (//\s@constructor\s(?P<constructors>.*?)\n)?
        (//\s@operator\s(?P<operators>.*?)\n)?
        (//\s@description\s(?P<description>.*?)\n)?
    """, re.VERBOSE)

    matches = pattern.finditer(cpp_code)

    lua_defs = []
    for match in matches:
        cls = match.group("class")
        fields = [match.group("fields"), match.group("morefields")]
        fields = [f for f in fields if f]
        fields = "\n".join(f"--- @field {line.strip()}" for f in fields for line in f.split())

        constructors = match.group("constructors") or ""
        cons_lines = []
        for ctor in constructors.split():
            args = ctor[ctor.find("(")+1:ctor.find(")")]
            arg_list = args.split(",") if args else []
            arg_list = [a.strip() for a in arg_list if a.strip()]
            args_lua = ", ".join(f"x{i}: number" for i, _ in enumerate(arg_list))
            cons_lines.append(f"--- @constructor fun({args_lua}): {cls}")

        operators = match.group("operators") or ""
        op_lines = [f"--- @operator {op_map(op)}({cls}|number): {cls}" for op in operators.split()]

        description = match.group("description") or ""

        lua_defs.append(f"""
            --- @class {cls} {fields} {chr(10).join(op_lines)} {chr(10).join(cons_lines)}
            -- {description}
            local {cls} = {{}}
        """)
    return "\n\n".join(lua_defs)

def op_map(op):
    return {
        "+": "add",
        "-": "sub",
        "*": "mul",
        "/": "div",
        "==": "eq"
    }.get(op, op)


if __name__ == "__main__":
    lua_stub = parse_lua_bindings(cpp_code)

    # Write to file
    with open("gen_vec2_stub.lua", "w") as f:
        f.write(lua_stub)

    print("Lua stub written to generated_vec2_stub.lua")
