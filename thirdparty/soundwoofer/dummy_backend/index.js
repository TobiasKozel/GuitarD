var http = require("http");
const fs = require("fs");
// let basePath = "./thirdparty/soundwoofer/dummy_backend/";
let basePath = "./";
let presets = [];

function log(s, req = false) {
    var ip = "";
    var d = new Date();
    var date = ("0" + d.getDate()).slice(-2) + "-" + ("0"+(d.getMonth()+1)).slice(-2) + "-" +
        d.getFullYear() + " " + ("0" + d.getHours()).slice(-2) + ":" + ("0" + d.getMinutes()).slice(-2);
    if (req) {
        ip = req.headers['x-forwarded-for'] || req.connection.remoteAddress || 
            req.socket.remoteAddress || (req.connection.socket ? req.connection.socket.remoteAddress : null);
    }
    if (ip) {
        console.log(date + " | " + ip + " | " + s);
    }
    else {
        console.log(date + " | " + s);
    }
}

class Preset {
    constructor(name, data) {
        if (name.includes(".json")) {
            this.name = name.replace(/.json/, "");
        }
        else {
            this.name = name;
        }
        this.id = name;
        this.rating = 0;
        this.ratings = 0;
        this.user = "";
        this.data = undefined;
        this.changed = false;
        this.version = "0";
        this.plugin = "GuitarD";
    }
}

var PresetManager = {
    presets: [],
    getPresetList: function() {
        let files = [];
        fs.readdirSync(basePath + "presets/").forEach(file => {
            if (file.includes(".json")) {
                files.push(file);
            }
        });
        return files;
    },

    init: function() {
        let files = this.getPresetList(); // get all the presets in the folder
        let db = JSON.parse(fs.readFileSync(basePath + "presets.json")); // get all index presets
        let list = [];
        for (let i of files) {
            let data = fs.readFileSync(basePath + "presets/" + i, "utf8");
            var indexed = false;
            for (let j of db) {
                if (i === j.id) {
                    j.data = data;
                    indexed = true;
                    list.push(j);
                    break;
                }
            }
            if (indexed == false) {
                list.push(new Preset(i, data));
            }
        }
        this.presets = list;
        this.commit();
    },

    commit: function() {
        for (let i of this.presets) {
            if (i.changed) {
                fs.writeFileSync(basePath + "presets/" + i.id , i.data);
            }
        }
        let list = [];
        for (let i of this.presets) {
            let j = Object.assign({}, i);;
            delete j.data;
            delete j.changed;
            list.push(j);
        }
        fs.writeFileSync(basePath + "presets.json" , JSON.stringify(list));
    },

    add: function(preset) {
        for (let i of this.presets) {
            if (i.id === preset.id) {
                i.data = preset.data;
                this.commit();
                return;
            }
        }
        p = new Preset(preset.name, preset.data);
        this.presets.push(p);
        this.commit();
    }
}

PresetManager.init();

http.createServer((req, res) => {
    if (req.method == "POST") {
        /**
         * Preset was send
         */
        if (req.url.search("/Preset") === 0) {
            var body = "";
            req.on("data", (data) => {
                body += data;
            });
            req.on("end", () => {
                console.log("Saving Preset");
                PresetManager.add(JSON.parse(body))
                res.writeHead(200, {"Content-Type": "text/html"});
                res.end();
            });
        }
    }
    if (req.method == "GET") {
        var body = "";
        if (req.url.search("/Preset") === 0) {
            // var waitTill = new Date(new Date().getTime() + 4 * 1000);
            // while(waitTill > new Date()){}
            body = JSON.stringify(PresetManager.presets);
            log("Fetching presets...", req);
        }

        if (req.url.search("/Impulse") === 0) {
            let data = JSON.parse(fs.readFileSync(basePath + "irs.json"));
            body = JSON.stringify(data);
            log("Fetching IR list...", req);
        }

        if (req.url.search("/Component") === 0) {
            let data = JSON.parse(fs.readFileSync(basePath + "components.json"));
            body = JSON.stringify(data);
            log("Fetching Components...", req);
        }

        if (req.url.search("/Rig") === 0) {
            let data = JSON.parse(fs.readFileSync(basePath + "rigs.json"));
            body = JSON.stringify(data);
            log("Fetching Rigs...", req);
        }
        if (req.url.search("/File") === 0) {
            let id = req.url.slice("/File/Download/".length);

            let data;
            try {
                data = fs.readFileSync(basePath + "irs/" + id);
                log("Fetching IR File...", req);
                res.writeHead(200, {"Content-Type": "application/octet-stream"});
                res.write(data, "binary");
                res.end(null, "binary");
            } catch(err) {
                log("error loading ir file", req);
                res.writeHead(200, {"Content-Type": "application/json"});
                res.end(body);
            }
            
        }
        else {
            res.writeHead(200, {"Content-Type": "application/json"});
            res.end(body);
        }
    }
}).listen(55554); 
