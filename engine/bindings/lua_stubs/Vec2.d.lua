--- @meta

--- @class Vec2
--- @field x number
--- @field y number
--- @operator add(Vec2|number): Vec2
--- @operator sub(Vec2|number): Vec2
--- @operator mul(Vec2|number): Vec2
--- @operator div(Vec2|number): Vec2
--- @operator unm: Vec2
local Vec2 = {}
Vec2.__index = Vec2
setmetatable(Vec2, {
    __call = function(_, x, y)
        assert((x == nil and y == nil) or (type(x) == type(y)), "x and y must have same type")
        return setmetatable({
            x = x or 0,
            y = y or 0
        }, Vec2)
    end
})

return Vec2