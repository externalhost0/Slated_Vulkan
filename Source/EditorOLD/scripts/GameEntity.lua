---@class GameEntity
---@field name string
---@field health number
local GameEntity = {}
GameEntity.__index = GameEntity

---@return GameEntity
function GameEntity.new()
    local self = setmetatable({}, GameEntity)
    self.name = "Unnamed"
    self.health = 100
    return self
end

---@param dt number
function GameEntity:update(dt)
    print(self.name .. " updated by " .. dt)
end

---@param damage number
function GameEntity:takeDamage(damage)
    self.health = self.health - damage
end

return GameEntity
