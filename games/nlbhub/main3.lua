-- $Name:Non-Linear Text Quests$
-- $Version: 1.0$
-- $Author:NLB project$
-- $Info:Collection of text-based games\nby different authors$

package.path = "lua/?.lua;" .. instead_cwdpath() .. "/lua/?.lua;" .. package.path;
package.cpath = "?.dll;" .. instead_cwdpath() .. "/?.so;" .. package.cpath;

require 'nlbhub'
require 'utils'
require 'sprite'
loadmod 'decor'
loadmod 'hyperlink'

const {
    LAUNCHER_NAME = "Non-Linear Text Quests";
}

instead.nosave = true;
instead.noautosave = true;

global 'title_font' '';  -- see init()
global 'title_color' 'white';

function init()
    theme.set('scr.gfx.bg', 'gfx/bg.png');
    change_pl(pl_nlbproject);
    init_hub(function(g, properties)
        -- NB: You shouldn't enable saves here, because autosave will be written at this point
        --instead.nosave = false;
        --instead.noautosave = false;
        if g.standalone or properties.nlbproject then
            loadgame(g.name);
        else
            loadgame_safe(g.name);
        end
    end);
    gdesc:disable();
    title_font = sprite.fnt('fonts/STEINEMU.ttf', 48);
end

declare 'f_cache_size' (function()
    return "Cache size: " .. tostring(nicesize(here().cache_size)) .. " ({clear_cache_cmd|clear})";
end)

declare 'f_controls' (function()
    local config = load_config();
    local mark = "• ";
    local nlbproject = get_lang() == "ru" and [[: ({nlbproject_en|EN}) | ◄({nlbproject_ru|RU})►
    ]] or [[: ◄({nlbproject_en|EN})► | ({nlbproject_ru|RU})
    ]];
    local community = get_lang() == "ru" and [[: ({community_en|EN}) | ◄({community_ru|RU})►
    ]] or [[: ◄({community_en|EN})► | ({community_ru|RU})
    ]];
    -- {nlbproject_all|all} , {community_all|all} filters were removed from UI but should still work
    local reponame1 = here().list_name == 'nlbproject' and mark .. config["nlbhub.general.main-repository-name"] or config["nlbhub.general.main-repository-name"];
    local reponame2 = here().list_name == 'community' and mark .. config["nlbhub.general.community-repository-name"] or config["nlbhub.general.community-repository-name"];
    return reponame1 .. nlbproject .. reponame2 .. community;
end)

function start(load)
    D {
        "controls",
        "txt",
        f_controls,
        x = 37,
        y = 55,
        w = 500,
        align = 'right',
        hidden = false,
        typewriter = false,
        z = -1
    };
    -- appdata dir MUST exist near instead.exe!
    mkdir(get_gamesdir());
    mkdir(get_cachedir());
    here().cache_size = get_dir_size(get_cachedir());
    D {
        "cache_stats",
        "txt",
        f_cache_size,
        x = 647,
        y = 55,
        w = 1177,
        align = 'center',
        hidden = false,
        typewriter = false,
        z = -1
    };
    theme.set('scr.gfx.bg', 'gfx/bg.png');
    here().desc_nlbproject["en"] = get_desc("desc_nlbproject_en.txt");
    here().desc_nlbproject["ru"] = get_desc("desc_nlbproject_ru.txt");
    here().desc_community["en"] = get_desc("desc_community_en.txt");
    here().desc_community["ru"] = get_desc("desc_community_ru.txt");
    local g = stead.ref(here().selectedGame);
    if not g then
        here().selectedGame = '';
    end
end
   
function game:ondecor(name, press, x, y, btn, act, a, b)
    theme.set('scr.gfx.bg', 'gfx/bg.png');
    if not act then
        return false;
    end
    if act == 'clear_cache_cmd' then
        delete_all_subdirs(get_cachedir());
        here().cache_size = 0;
        instead_busy(false);
        D(D"cache_stats");
        return true;
    end
    gdesc:disable();
    here().lang = act:match("_(.*)$");
    here().selectedGame = '';
    here().list_name = 'nlbproject';
    for i, v in ipairs(here().games_list) do
        v:toggle();
    end
    if act:starts('nlbproject') then
        D(D'controls');
        change_pl(pl_nlbproject);
        return;
    elseif act:starts('community') then
        here().list_name = 'community';
        D(D'controls');
        change_pl(pl_community);
        return;
    end
    return false;
end

game.display = stead.hook(game.display, function(f, s, state, ...)
    theme.set('scr.gfx.bg', 'gfx/bg.png');
    return f(s, state, ...);
end)

pl_nlbproject = player {
    nam = "pl_nlbproject";
    room = 'main';
};

pl_community = player {
    nam = "pl_community";
    room = 'main';
};

delete = menu {
   nam = "@delete",
   system_type = true,
   act = function(s)
       gdesc.confirm_delete_mode = false;
       local g = stead.ref(here().selectedGame);
       g:delete();
   end,
   menu = function(s)
       return s:act();
   end
}

launcher = menu {
   nam = "@launcher",
   system_type = true,
   act = function(s)
       local g = stead.ref(here().selectedGame);
       if g then
           g:lnch();
       end
   end,
   menu = function(s)
       return s:act();
   end
}

gdesc = menu {
    nam = "gdesc",
    system_type = true,
    confirm_delete_mode = false,
    pic = function(s)
        if not here().selectedGame then return "" end
        local g = stead.ref(here().selectedGame);
        if not g then return "" end
        local screenshot_file, exists = g:get_pic_file();
        if exists then
            return fmt.img(screenshot_file);
        else
            return "";
        end
    end,
    ctrls = function(s)
        local g = stead.ref(here().selectedGame);
        local result;
        if get_lang() == "ru" then
            result = g:exists() and (s.confirm_delete_mode and "Действительно удалить файлы этой игры с диска? {@delete|Да} | {Нет}" or "{Удалить} | {@launcher|Начать}") or "{@launcher|Скачать}" .. g:get_size_string();
        else
            result = g:exists() and (s.confirm_delete_mode and "Really delete this game files from disk? {@delete|Yes} | {No}" or "{Delete} | {@launcher|Start}") or "{@launcher|Download}" .. g:get_size_string();
        end
        return result;
    end,
    act = function(s)
        s.confirm_delete_mode = not s.confirm_delete_mode;
    end,
    menu = function(s)
        return s:act();
    end,
    dsc = function(s)
        if here().selectedGame ~= '' then
            local g = stead.ref(here().selectedGame);
            local result = g:desc();
            pn(fmt.c(s:ctrls()));
            pn(result);
            pn();
            pn(fmt.c(s:pic()));
        end
    end
};

instead.get_title = std.cacheable('title', function()
    return iface:fmt(fmt.img(title_font:text(std.titleof(stead.here()), title_color)), false)
end)


main = room {
    nam = "main",
    {
        games_list = {}
    },
    selectedGame = '',
    list_name = 'nlbproject',
    lang = 'all',
    cache_size = 0,
    desc_nlbproject = {},
    desc_community = {},
    disp = function(s)
        if s.selectedGame == '' then
            return LAUNCHER_NAME;
        else
            local g = stead.ref(s.selectedGame);
            return g and g:disp_txt() or LAUNCHER_NAME;
        end
    end,
    decor = function(s)
        local result = nil;
        if s.selectedGame == '' then
            result = s.list_name == 'nlbproject' and s.desc_nlbproject[get_lang()] or s.desc_community[get_lang()];
        end
        return result;
    end,
    obj = {"gdesc"}
};
