require "fmt"
local statsAPI = require "luapassing"

if not instead.atleast(3, 2) then
    std.dprint("Warning: hyperlink module is not functional on this INSTEAD version")
end

obj {
    nam = '@hyperlink';
    act = function(s, w)
        statsAPI.openURL(w)
    end;
}

std.mod_init(function()
    statsAPI.init();
end)
