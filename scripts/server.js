const http = require("http");
const fs = require("fs");
const path = require("path");

http.createServer(function (request, response) {
  console.log(`Requesting ${request.url}...`);

  let filePath = "." + request.url;
  if (filePath == "./") filePath = "./index.html";

  const extname = String(path.extname(filePath)).toLowerCase();
  const mimeTypes = {
      ".html": "text/html",
      ".js": "text/javascript",
      ".css": "text/css",
      ".png": "image/png",
      ".wasm": "application/wasm",
  };

  const contentType = mimeTypes[extname] || "application/octet-stream";

  fs.readFile(path.join("wasm", filePath), function (error, content) {
    if (error) {
      if (error.code == "ENOENT") {
        response.writeHead(404, { "Content-Type": "text/html" });
        response.end("Not found...\n");
        response.end();
      }
      else {
        response.writeHead(500, { "Content-Type": "text/html" });
        response.end("Sorry, check with the site admin for error: " + error.code + " ..\n");
        response.end();
      }
    }
    else {
      response.writeHead(200, { "Content-Type": contentType });
      response.end(content, "utf-8");
    }
  });

}).listen(8000);
console.log("Server running at http://127.0.0.1:8000/");