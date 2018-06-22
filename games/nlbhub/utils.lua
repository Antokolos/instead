local lfs = require('lfs');

function mkdir(dir)
    return lfs.mkdir(dir);
end

function get_dir_size(dir)
    local result = 0;
    for file in lfs.dir(dir) do
        instead_busy(true);
        local file_path = dir..'/'..file
        if file ~= "." and file ~= ".." then
            if lfs.attributes(file_path, 'mode') == 'file' then
                result = result + lfs.attributes(file_path, 'size');
            elseif lfs.attributes(file_path, 'mode') == 'directory' then
                result = result + get_dir_size(file_path);
            end
        end
    end
    return result;
end

function deletedir(dir)
    for file in lfs.dir(dir) do
        instead_busy(true);
        local file_path = dir..'/'..file
        if file ~= "." and file ~= ".." then
            if lfs.attributes(file_path, 'mode') == 'file' then
                os.remove(file_path)
                --print('remove file', file_path)
            elseif lfs.attributes(file_path, 'mode') == 'directory' then
                --print('dir', file_path)
                deletedir(file_path)
            end
        end
    end
    return lfs.rmdir(dir)
    --print('remove dir', dir)
end

function delete_all_subdirs(dir)
    for file in lfs.dir(dir) do
        instead_busy(true);
        local file_path = dir..'/'..file
        if file ~= "." and file ~= ".." then
            if lfs.attributes(file_path, 'mode') == 'directory' then
                deletedir(file_path)
            end
        end
    end
end

function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function string.ends(String,End)
   return End=='' or string.sub(String,-string.len(End))==End
end

function load_config()
    local file = assert(io.open("nlbhub.ini", "r"))
    local config = {}
    for line in file:lines() do
        for key, value in string.gmatch(line, "(.*)=(.*)") do
            config[trim(key)] = trim(value);
        end
    end
    return config;
end

function trim(s)
    return (string.gsub(s, "^%s*(.-)%s*$", "%1"))
end

function instead_trim(s)
    return (string.gsub(s, "^[%s%^]*(.-)[%s%^]*$", "%1"))
end

function clone(t) -- deep-copy a table
    if type(t) ~= "table" then return t end
    local meta = getmetatable(t)
    local target = {}
    for k, v in pairs(t) do
        if type(v) == "table" then
            target[k] = clone(v)
        else
            target[k] = v
        end
    end
    setmetatable(target, meta)
    return target
end

function copy(t) -- shallow-copy a table
    if type(t) ~= "table" then return t end
    local meta = getmetatable(t)
    local target = {}
    for k, v in pairs(t) do target[k] = v end
    setmetatable(target, meta)
    return target
end