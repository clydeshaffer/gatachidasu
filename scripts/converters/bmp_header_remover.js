const fs = require('fs');
const path = require('path');
const argv = require('minimist')(process.argv.slice(2));
const BMP = require('bitmap-manipulation');

const randchars = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNNM1234567890";
function rndStr(len) {
    var out = "";
    for(var i = 0; i < len; i++) {
        out += randchars[Math.floor(Math.random() * randchars.length)];
    }
    return out;
}

function flipV(inputImage) {
    let flippedImage = new BMP.BMPBitmap(inputImage.width, inputImage.height, 1);
    for(var i = 0; i < inputImage.getHeight(); i++) {
        flippedImage.drawBitmap(inputImage, 0, i, null, 0, inputImage.height - i - 1, inputImage.width, inputImage.height);
    }
    return flippedImage;
}

function saveHeadless(img, filename) {
    var tmpname = rndStr(16) + ".tmp";
    var pixelCount = img.width * img.height;
    img.save(tmpname);
    const tmpFileSize = fs.statSync(tmpname).size;

    const parentDir = path.dirname(filename);
    if(!fs.existsSync(parentDir)) {
        fs.mkdirSync(parentDir, {recursive : true});
    }

    const fd = fs.openSync(tmpname, 'r');
    
    // Allocate a buffer to read 4 bytes
    const buffer = Buffer.alloc(4);

    // Read 4 bytes from the specified offset
    fs.readSync(fd, buffer, 0, 4, 0xA);

    const dataOffset = buffer.readUInt32LE(0);

    const outputStream = fs.createWriteStream(filename);
    outputStream.on("finish", () => {
        fs.unlinkSync(tmpname);
    });
    fs.createReadStream(tmpname, { start : dataOffset, end : dataOffset + pixelCount}).pipe(outputStream);
}

const inFileName = argv._[0];
const outFileName = argv._.length == 2 ?
    argv._[1] :
    inFileName.split(".").slice(0, -1).join(".") + ".gtg";

saveHeadless(
    flipV(BMP.BMPBitmap.fromFile(inFileName)),
    outFileName);