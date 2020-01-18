var http = require("http");
const fs = require("fs");
var presets = [
    {
        name: "Preset1",
        id: "id1",
        plugin: "GuitarD",
        data: "data1"
    },
    {
        name: "Preset2",
        id: "id2",
        plugin: "GuitarD",
        data: "data2"
    }
];

http.createServer((req, res) => {
    if (req.method == "POST") {
        if (req.url.search("/Preset") === 0) {
            var body = "";
            req.on("data", (data) => {
                body += data;
            });
            req.on("end", () => {
                presets.push(JSON.parse(body));
                res.writeHead(200, {"Content-Type": "text/html"});
                res.end();
            });
        }
    }
    if (req.method == "GET") {
        var body = "";
        if (req.url.search("/Preset") === 0) {
            var waitTill = new Date(new Date().getTime() + 4 * 1000);
            while(waitTill > new Date()){}
            body = JSON.stringify(presets);
        }

        if (req.url.search("/Impulse") === 0) {
            let data = JSON.parse(fs.readFileSync("./thirdparty/soundwoofer/dummy_backend/Impulse.json"));
            body = JSON.stringify(data);
        }

        if (req.url.search("/Rig") === 0) {
            let data = JSON.parse(fs.readFileSync("./thirdparty/soundwoofer/dummy_backend/Rig.json"));
            body = JSON.stringify(data);
        }
        res.writeHead(200, {"Content-Type": "application/json"});
        res.end(body);
    }
}).listen(5000); 