local socket = require("socket");
local https = require('ssl.https');
https.TIMEOUT = 20;
local http = require('socket.http');
http.TIMEOUT = 20;
local ltn12 = require('ltn12');

local function get_http_obj(url)
    if url:sub(1, 5) == 'https' then
        return https;
    else
        return http;
    end
end

-- formats a number of seconds into human readable form
function nicetime(s)
    local l = "s"
    if s > 60 then
        s = s / 60
        l = "m"
        if s > 60 then
            s = s / 60
            l = "h"
            if s > 24 then
                s = s / 24
                l = "d" -- hmmm
            end
        end
    end
    if l == "s" then return string.format("%5.0f%s", s, l)
    else return string.format("%5.2f%s", s, l) end
end

-- formats a number of bytes into human readable form
function nicesize(b)
    local l = "B"
    if b > 1024 then
        b = b / 1024
        l = "KB"
        if b > 1024 then
            b = b / 1024
            l = "MB"
            if b > 1024 then
                b = b / 1024
                l = "GB" -- hmmm
            end
        end
    end
    return string.format("%7.2f%2s", b, l)
end

-- returns a string with the current state of the download
local remaining_s = "%s received, %s/s throughput, %2.0f%% done, %s remaining"
local elapsed_s =   "%s received, %s/s throughput, %s elapsed                "
local function gauge(got, delta, size)
    local rate = got / delta
    if size and size >= 1 then
        return string.format(remaining_s, nicesize(got),  nicesize(rate),
            100*got/size, nicetime((size-got)/rate))
    else
        return string.format(elapsed_s, nicesize(got),
            nicesize(rate), nicetime(delta))
    end
end

-- creates a new instance of a receive_cb that saves to disk
-- kind of copied from luasocket's manual callback examples
local function stats(size)
    local start = socket.gettime()
    local last = start
    local got = 0
    return function(chunk)
        instead_busy(true);
        -- elapsed time since start
        local current = socket.gettime()
        if chunk then
            -- total bytes received
            got = got + string.len(chunk)
            -- not enough time for estimate
            if current - last > 1 then
                print("\r", gauge(got, current - start, size));
                last = current
            end
        else
            -- close up
            print("\r", gauge(got, current - start), "\n")
        end
        return chunk
    end
end

-- determines the size of a http file
function gethttpsize(url)
    local httpObj = get_http_obj(url);
    local r, c, h = httpObj.request {method = "HEAD", url = url}
    if c == 200 then
        return tonumber(h["content-length"])
    end
end

function download_to_string(url)
    local str = '';
    local httpObj = get_http_obj(url);
    local r, c, h, s = httpObj.request {
        url = url,
        sink = function(chunk, err)
            if not chunk then
                return 1
            end
            str = str .. chunk
            return chunk:len()
        end
    }
    return str, r, c, h, s;
end

function download_to_file(url, fname, fsize)
    local file = io.open(fname, 'wb');
    local save = ltn12.sink.file(file or io.stdout);
    local httpObj = get_http_obj(url);
    local r, c, h, s = httpObj.request {
        url = url,
        sink = ltn12.sink.chain(stats(fsize or gethttpsize(url)), save)
    };
    instead_busy(false);
    if c == 301 then
        -- file:close() is not needed; already closed
        os.remove(fname);
        local redirect_html = download_to_string(url);
        local redirect_url = redirect_html:match("<a href=['\"](.*)['\"]>");
        if redirect_url then
            return download_to_file(redirect_url, fname, fsize);
        else
            print("HTTP code 301, but we failed to get redirect url, HTML response was:", "\n");
            print(redirect_html, "\n");
        end
    elseif c ~= 200 then
        print(s or c, "\n");
        -- file:close() is not needed; already closed
    end
    return r, c, h, s;
end

if stead.mod_init then
    stead.mod_init(function()
    end)
end

if stead.module_init then
    stead.module_init(function()
    end)
end