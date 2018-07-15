require 'fmt'
require 'network'
require 'utils'
local SLAXML = require('slaxml');

function get_lang()
    if here().lang == "all" then
        return "en";
    end
    return here().lang;
end

function get_cachedir()
    return instead_appdirpath() .. "cache";
end

function get_gamesdir()
    return instead_appdirpath() .. "games";
end

function games_feed(games, baseurl, url, lang)
    local xml, r, c, h, s = download_to_string(url);
    local tag;
    local val = {};
    if xml == '' then
        return r, c, h, s;
    end
    local parser = SLAXML:parser {
        startElement = function(name, nsURI,nsPrefix)
            instead_busy(true);
	    if name == 'game' then
                tag = false;
                val = {};
            else
                tag = name;
            end
	end, -- When "<foo" or <x:foo is seen
        attribute  = function(name,value,nsURI,nsPrefix)
        end, -- attribute found on current element
        closeElement = function(name,nsURI)
            instead_busy(true);
	    if name == 'game' then
                local g = games[val.name];
                if g then
                    g.title[lang] = val.title;
                    g.descurl[lang] = val.descurl;
                else
                    local title = {};
                    title[lang] = val.title;
                    local descurl = {};
                    descurl[lang] = val.descurl;
                    games[val.name] = {
                        name = val.name,
                        url = val.url,
                        title = title,
                        size = tonumber(val.size),
                        version = val.version,
                        lang = val.lang,
                        standalone = val.standalone,
                        statid = val.statid,
                        descurl = descurl,
                        baseurl = baseurl
                    };
                end
            else
                tag = false;
            end
        end, -- When "</foo>" or </x:foo> or "/>" is seen
        text = function(text)
            instead_busy(true);
            if tag and not val[tag] then
                val[tag] = text;
            end
        end, -- text and CDATA nodes
        comment = function(content)
        end, -- comments
        pi = function(target,content)
        end, -- processing instructions e.g. "<?yes mon?>"
    }
    parser:parse(xml, {stripWhitespace = true});
    instead_busy(false);
    return r, c, h, s;
end

get_names = function(f)
    local mainf = io.open(get_gamesdir() .. "/" .. f .. "/" .. "main.lua", "r") or io.open(get_gamesdir() .. "/" .. f .. "/" .. "main3.lua", "r");
    local nameen = nil;
    local nameru = nil;
    if mainf then
        while true do
            local line = mainf:read("*l")
            if line == nil then break end;
            if not nameen then
                nameen = line:match("^%-%-%s*%$Name:%s*([^%$]+)%$");
            end
            if not nameru then
                nameru = line:match("^%-%-%s*%$Name%(ru%):%s*([^%$]+)%$");
            end
            if nameen and nameru then
                break
            end
        end
        mainf:close();
        --nameru = nameru or nameen;
    end
    return {["ru"] = nameru, ["en"] = nameen};
end;

get_desc = function(desc_file)
    local descf = io.open(desc_file, "r");
    if descf then
        local d = descf:read("*all");
        descf:close();
        return d;
    end
    return nil;
end;

get_real_descs = function(game_dir)
    local descen = get_desc(get_gamesdir() .. "/" .. game_dir .. "/" .. "desc_en.txt");
    local descru = get_desc(get_gamesdir() .. "/" .. game_dir .. "/" .. "desc_ru.txt");
    return {["ru"] = descru, ["en"] = descen};        
end;

get_cached_descs = function(game_dir)
    local descen = get_desc(get_cachedir() .. "/" .. game_dir .. "/" .. "desc_en.txt");
    local descru = get_desc(get_cachedir() .. "/" .. game_dir .. "/" .. "desc_ru.txt");
    return {["ru"] = descru, ["en"] = descen};
end;

get_descs = function(game_dir)
    local descs = get_real_descs(game_dir);
    if descs["ru"] and descs["en"] then
        return descs;
    end
    local cached = get_cached_descs(game_dir);
    if not descs["ru"] then
        descs["ru"] = cached["ru"];
    end
    if not descs["en"] then
        descs["en"] = cached["en"];
    end
    return descs;
end;

local function basename(str)
    local name = string.gsub(str, "(.*/)(.*)", "%2");
    return name;
end

local function sanitize(str)
    local result = str:gsub("&laquo;", "«");
    result = result:gsub("&raquo;", "»");
    result = result:gsub("&#034;", "\"");
    result = result:gsub("&#039;", "'");
    result = result:gsub("&#151;", "—");
    result = result:gsub("&quot;", "\"");
    result = result:gsub("&gt;", ">");
    result = result:gsub("&lt;", "<");
    return result;
end

game_entry = function(g, launch_func, properties)
    local name = g.name;
    local cached_disp = {};
    local cached_disp_txt = {};
    local cached_desc = {};
    local v = {};
    v.nam = name;
    v.properties = copy(properties);
    v.disp_txt = function(s)
        if cached_disp_txt[get_lang()] then
            return cached_disp_txt[get_lang()];
        end
        cached_disp_txt[get_lang()] = sanitize(get_names(name)[get_lang()] or g.title[get_lang()] or name);
        return cached_disp_txt[get_lang()];
    end;
    v.disp = function(s)
        if cached_disp[get_lang()] then
            return cached_disp[get_lang()];
        end
        cached_disp[get_lang()] = (s.properties.installed and fmt.img('gfx/start.png') or fmt.img('gfx/avail.png')) .. s:disp_txt();
        return cached_disp[get_lang()];
    end;
    v.desc = function(s)
        if cached_desc[get_lang()] then
            return cached_desc[get_lang()];
        end
        local save_desc_cache = function(desc_text)
            local dname = get_cachedir() .. "/" .. g.name;
            lfs.mkdir(dname);
            local fname = dname .. "/desc_" .. get_lang() .. ".txt";
            lfs.touch(fname);
            local file = io.open(fname, "a");
            file:write(desc_text);
            file:close();
            here().cache_size = here().cache_size + lfs.attributes(fname, 'size');
        end;
        local save_screenshot_cache = function(html)
            local screenshots_html = html:match("<div id=\"screenshots\">(.-)</div>");
            if screenshots_html then
                local screenshot_url = screenshots_html:match("<a rel=\"shadowbox\" href=\"(.-)\">");
                local pic_file, exists = s:get_pic_file();
                if screenshot_url and not exists then
                    download_to_file(g.baseurl .. "/" .. screenshot_url, pic_file);
                    here().cache_size = here().cache_size + lfs.attributes(pic_file, 'size');
                end
            end
        end
        local parsedesc = function(default_desc)
            if not g.descurl then
                return default_desc;
            end
            local url = g.descurl[get_lang()];
            if not url then
                return default_desc;
            end
            local html = download_to_string(url);
            local clean_CR = html:gsub("\r", "");
            local clean_NL = clean_CR:gsub("\n", "");
            local desc_all = clean_NL:match("<div class=\"gamedsc\">(.-)</div>");
            if not desc_all then
                return default_desc;
            end

            local clean_BR = desc_all:gsub("<br.->", "^");
            local result = clean_BR:gsub("%^+", "^");
            result = instead_trim(sanitize(result:gsub("<.->", "")));
            save_desc_cache(result);
            save_screenshot_cache(html);
            D(D"cache_stats");
            return result;
        end;
        cached_desc[get_lang()] = get_descs(name)[get_lang()] or parsedesc(s:disp_txt());
        return cached_desc[get_lang()];
    end;
    v.lnch = function(s)
        local main_lua = io.open(get_gamesdir() .. '/' .. name .. '/main.lua', 'r') or io.open(get_gamesdir() .. '/' .. name .. '/main3.lua', 'r');
        if main_lua == nil then
            -- game directory does not contain main game file
            local fname = get_cachedir() .. "/" .. basename(g.url);
            download_to_file(g.url, fname, g.size);
            --installgame(fname, instead_gamespath(), name);
            installgame(fname, get_gamesdir(), name);
            os.remove(fname);
            s.properties.installed = true;
            s:clear_txt_cache();
        else
            main_lua:close();
            launch_func(g, s.properties);
        end
    end;
    v.clear_txt_cache = function(s)
        cached_disp = {};
        cached_disp_txt = {};
        cached_desc = {};
    end;
    v.clear_cache = function(s)
        s:clear_txt_cache();
        local cache_dir = get_cachedir() .. "/" .. name;
        if (lfs.attributes(cache_dir, 'mode') == 'directory') then
            here().cache_size = here().cache_size - get_dir_size(cache_dir);
            deletedir(cache_dir);
            instead_busy(false);
        end
    end;
    v.delete = function(s)
        if s:exists() then
            deletedir(get_gamesdir() .. '/' .. name);
            instead_busy(false);
            s.properties.installed = false;
            if s.properties.unregistered then
                remove(s);
                here().selectedGame = '';
            end
            s:clear_cache();
        end
    end;
    v.menu = function(s)
        here().selectedGame = stead.deref(s);
        gdesc.confirm_delete_mode = false;
        gdesc:enable();
        return true;
    end;
    v.toggle = function(s)
        if here().lang == "all" or not g.lang or g.lang:find(here().lang) then
            s:enable();
        else
            s:disable();
        end
    end;
    v.exists = function(s)
        local main_lua = io.open(get_gamesdir() .. '/' .. name .. '/main.lua', 'r') or io.open(get_gamesdir() .. '/' .. name .. '/main3.lua', 'r');
        local exists = main_lua ~= nil;
        if exists then
            main_lua:close();
        end
        return exists;
    end;
    v.get_size_string = function(s)
        return g.size and nicesize(g.size) or "";
    end;
    -- Corresponding screenshot file and its existence flag
    v.get_pic_file = function(s)
        local screenshot_file = get_cachedir() .. "/" .. name .. "/screenshot.pic";
        return screenshot_file, lfs.attributes(screenshot_file, 'mode') == 'file';
    end;
    --v.act = function(s)
    --    return s.menu();
    --end
    return menu(v);
end

function init_hub(launch_func)
    local games_map = {};
    local config = load_config();
    local nlbproject_games = {};
    here().games_list = {};
    local r, c, h, s;
    local network_failure = false;
    r, c, h, s = games_feed(nlbproject_games, config["nlbhub.general.main-repository-baseurl"], config["nlbhub.general.main-repository-url"] .. "?lang=ru", "ru");
    if c ~= 200 then
        network_failure = true;
    end
    r, c, h, s = games_feed(nlbproject_games, config["nlbhub.general.main-repository-baseurl"], config["nlbhub.general.main-repository-url"] .. "?lang=en", "en");
    if c ~= 200 then
        network_failure = true;
    end
    for k, g in pairs(nlbproject_games) do
        here().has_nlbproject = true;
        instead_busy(true);
        -- Do not insert duplicates, if any
        if not games_map[g.name] then
            games_map[g.name] = game_entry(g, launch_func, {["nlbproject"] = true});
            stead.table.insert(here().games_list, games_map[g.name]);
        end
    end
    local community_games = {};
    r, c, h, s = games_feed(community_games, config["nlbhub.general.community-repository-baseurl"], config["nlbhub.general.community-repository-url"] .. "?lang=ru", "ru");
    if c ~= 200 then
        network_failure = true;
    end
    r, c, h, s = games_feed(community_games, config["nlbhub.general.community-repository-baseurl"], config["nlbhub.general.community-repository-url"] .. "?lang=en", "en");
    if c ~= 200 then
        network_failure = true;
    end
    for k, g in pairs(community_games) do
        here().has_community = true;
        instead_busy(true);
        if games_map[g.name] then
            games_map[g.name].properties.community = true;
        else
            games_map[g.name] = game_entry(g, launch_func, {["community"] = true});
            stead.table.insert(here().games_list, games_map[g.name]);
        end
    end
    for f in stead.readdir(get_gamesdir()) do
        instead_busy(true);
        if f ~= '.' and f ~= '..' and f ~= 'nlbhub' then
            if games_map[f] then
                games_map[f].properties.installed = true;
            else
                local g = {name = f, title = {["ru"] = f, ["en"] = f}};
                local params = {["community"] = true, ["installed"] = true, ["unregistered"] = true};
                games_map[f] = game_entry(g, launch_func, params);
                stead.table.insert(here().games_list, games_map[f]);
            end
        end
    end
    local sort_func = function(a, b)
        instead_busy(true);
        return a:disp_txt() < b:disp_txt();
    end;
    stead.table.sort(here().games_list, sort_func);
    change_pl(pl_nlbproject);
    inv():zap();
    change_pl(pl_community);
    inv():zap();
    for i, v in ipairs(here().games_list) do
        instead_busy(true);
        if v.properties.nlbproject then
            change_pl(pl_nlbproject);
        elseif v.properties.community then
            change_pl(pl_community);
        end
        take(v);
    end
    if network_failure then
        change_pl(pl_community);
        here().list_name = "community";
    else
        change_pl(pl_nlbproject);
    end
    D(D'nlb_controls');
    D(D'com_controls');
    -- Incrementing openstat counter
    download_to_string(config["nlbhub.general.openstat-url"] .. "?cid=" .. config["nlbhub.general.openstat-id"]);
    instead_busy(false);
end
