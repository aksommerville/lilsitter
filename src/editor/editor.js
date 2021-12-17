const VISUAL_SCALE = 4;
const spritesImage = new Image();
spritesImage.src = "/sprites.png";

/* Model.
 *********************************************************/
 
// main/map.h
const COMMANDS = [
// [name, opcode, argc (<0 if variable)]
  ["EOF",     0x00,  0],
  ["NOOP1",   0x01,  0],
  ["NOOPN",   0x02, -1], // (len,...)
  ["BGCOLOR", 0x03,  1], // (color)
  ["SOLID",   0x04,  4], // (x,y,w,h)
  ["ONEWAY",  0x05,  3], // (x,y,w)
  ["GOAL",    0x06,  0],
  ["HERO",    0x07,  2], // (x,y)
  ["DESMOND", 0x08,  2], // (x,y)
  ["SUSIE",   0x09,  2], // (x,y)
  ["FIRE",    0x0a,  2], // (x,y)
  ["DUMMY",   0x0b,  3], // (x,y,tileid)
];
 
const MAP_CMD_EOF           = 0x00;
const MAP_CMD_NOOP1         = 0x01;
const MAP_CMD_NOOPN         = 0x02;
const MAP_CMD_BGCOLOR       = 0x03;
const MAP_CMD_SOLID         = 0x04;
const MAP_CMD_ONEWAY        = 0x05;
const MAP_CMD_GOAL          = 0x06;
const MAP_CMD_HERO          = 0x07;
const MAP_CMD_DESMOND       = 0x08;
const MAP_CMD_SUSIE         = 0x09;
const MAP_CMD_FIRE          = 0x0a;
const MAP_CMD_DUMMY         = 0x0b;
 
class Map {
  constructor(bin) {
    this.bin = bin || new ArrayBuffer();
    this.decode();
  }
  
  decode() {
    this.commands = [];
    const u8s = new Uint8Array(this.bin);
    for (let p=0; p<u8s.length; ) {
      const cmd = (name, paylen) => {
        if (p > u8s.length - paylen) {
          throw new Error(`Short input around ${p}/${u8s.length}, expected ${paylen} bytes for ${name}`);
        }
        const command = Array.from(u8s.slice(p, p + paylen));
        command.splice(0, 0, name);
        this.commands.push(command);
        p += paylen;
      };
      switch (u8s[p++]) {
        case MAP_CMD_EOF: return;
        case MAP_CMD_NOOP1: cmd("NOOP1", 0); break;
        case MAP_CMD_NOOPN: cmd("NOOPN", 1 + u8s[p]); break;
        case MAP_CMD_BGCOLOR: cmd("BGCOLOR", 1); break;
        case MAP_CMD_SOLID: cmd("SOLID", 4); break;
        case MAP_CMD_ONEWAY: cmd("ONEWAY", 3); break;
        case MAP_CMD_GOAL: cmd("GOAL", 0); break;
        case MAP_CMD_HERO: cmd("HERO", 2); break;
        case MAP_CMD_DESMOND: cmd("DESMOND", 2); break;
        case MAP_CMD_SUSIE: cmd("SUSIE", 2); break;
        case MAP_CMD_FIRE: cmd("FIRE", 2); break;
        case MAP_CMD_DUMMY: cmd("DUMMY", 3); break;
        default: throw new Error(`Unknown command ${u8s[p-1]}`);
      }
    }
  }
  
  encode() {
    const len = this.commands.reduce((a, v) => a + v.length, 0);
    const u8s = new Uint8Array(len);
    let dstp = 0;
    for (const command of this.commands) {
      u8s[dstp++] = COMMANDS.find((c) => c[0] === command[0])[1];
      for (const v of command.slice(1)) {
        u8s[dstp++] = v;
      }
    }
    return u8s.buffer;
  }
}

/* Visual.
 ***********************************************************/

// common/ma_ctab.c
// grep '^_' src/editor/editor.js | sed -E 's/_\(([0-9a-f]{2}),([0-9a-f]{2}),([0-9a-f]{2})\)/"#\1\2\3",/g'
const kColorCssFromTiny = [
  "#000000","#450000","#ba0000","#ff0000","#002200","#452000","#b82400","#ff2400",
  "#004000","#454100","#ba4000","#ff4000","#006000","#456000","#ba6000","#ff5d00",
  "#009800","#459a00","#bd9d00","#ff9a00","#00bc00","#45b800","#babe00","#ffbe00",
  "#00e000","#4ddb00","#bfdb00","#ffe300","#00ff00","#4dff00","#b8ff00","#ffff00",
  "#000020","#450024","#ba0024","#ff0024","#002424","#452024","#bc2028","#ff2424",
  "#004124","#414224","#ba4424","#ff4124","#006222","#426222","#bf6024","#ff651c",
  "#00a224","#459a24","#bc9a24","#ff9a24","#00be24","#45be24","#beb824","#ffbe24",
  "#00db24","#4ddb24","#bcdc24","#ffdb24","#00ff24","#4dff24","#c0ff24","#ffff24",
  "#000041","#450041","#ba0041","#ff0041","#001c41","#451c41","#ba1c41","#ff2441",
  "#004040","#404040","#ba4141","#ff4040","#005d41","#456040","#b85e44","#ff5d41",
  "#009f41","#409c40","#bc9c44","#ff9a41","#00be41","#45be41","#bdbc45","#ffbe41",
  "#00db49","#3de341","#bad840","#ffdc44","#00ff49","#4dff41","#b8ff40","#ffff41",
  "#000065","#400065","#ba0065","#ff0065","#002465","#451c65","#ba1c65","#ff2460",
  "#004165","#454165","#ba4165","#ff4165","#006060","#406560","#bc6265","#ff6060",
  "#00a265","#459c65","#ba9c65","#ff9c65","#00be65","#45be65","#b8be65","#ffbe65",
  "#00db6d","#45db65","#bcdc65","#ffdc64","#00ff6d","#45ff60","#baff65","#ffff65",
  "#00009a","#400098","#ba009a","#ff1898","#00249a","#40249a","#ba249a","#ff2898",
  "#004992","#45419a","#ba449a","#ff4498","#006592","#456098","#ba659a","#ff5d9a",
  "#009a9a","#459a9a","#a09c9c","#ff9898","#00be92","#4cb89a","#babe9a","#ffbe92",
  "#00db9a","#4bd898","#bae39a","#ffdb92","#00ff92","#45ffa4","#baff9a","#ffff9a",
  "#001eb6","#4000be","#ba20b6","#ff00b6","#0024b6","#4028be","#ba24b6","#ff24b6",
  "#0049b6","#4044b8","#ba44b8","#ff44b8","#0065b6","#4565b4","#ba6db6","#ff64b8",
  "#009abe","#4098be","#ba9abe","#ff9abe","#00bebe","#4ababb","#babebe","#ffbcbc",
  "#00e3be","#45e3be","#bcdeba","#ffdcb8","#00ffb6","#45ffbe","#baffb6","#ffffbe",
  "#0000db","#4010db","#ba00db","#ff14db","#0024db","#402cd7","#ba24db","#ff24db",
  "#0047d7","#4541db","#ba41db","#ff49db","#0065d3","#4565db","#ba65db","#ff65db",
  "#009ae3","#459adb","#baa2db","#ffa2db","#00bedb","#40bedb","#babedb","#ffb6db",
  "#00dbdb","#44dbdb","#badbdb","#f8dede","#00ffdb","#4dffdb","#baffdb","#ffffdb",
  "#0000ff","#4400ff","#ba00ff","#ff00ff","#0024ff","#4024ff","#ba24ff","#ff24ff",
  "#0041ff","#4444ff","#ba49ff","#ff40ff","#0065ff","#425cff","#ba65ff","#ff65ff",
  "#009aff","#489aff","#baa0ff","#ffa0ff","#00b6ff","#4db6ff","#bac4ff","#ffc4ff",
  "#00dbff","#45dbff","#b8e0ff","#ffe4ff","#00ffff","#4dffff","#baffff","#ffffff",
];
 
function colorCssFromTiny(tiny) {
  return kColorCssFromTiny[tiny & 0xff];
}
 
function drawMap(map) { // Map

  const canvas = document.getElementById("visual");
  const context = canvas.getContext("2d");
  context.fillStyle = "#000";
  context.fillRect(0, 0, canvas.width, canvas.height);

  let gotBgcolor = false, gotOther = false;
  let goal = false;
  const warn = (msg) => {
    document.getElementById("warnings").innerText += msg + '\n';
  };
  for (const command of map.commands) {
    switch (command[0]) {
    
      case "BGCOLOR": {
          if (gotBgcolor) warn("Multiple BGCOLOR");
          if (gotOther) warn("BGCOLOR after other rendering");
          gotBgcolor = true;
          context.fillStyle = colorCssFromTiny(command[1]);
          context.fillRect(0, 0, canvas.width, canvas.height);
        } break;
        
      case "SOLID": {
          if (!gotBgcolor) warn("SOLID before BGCOLOR");
          gotOther = true;
          const x = command[1] * VISUAL_SCALE;
          const y = command[2] * VISUAL_SCALE;
          const w = command[3] * VISUAL_SCALE;
          const h = command[4] * VISUAL_SCALE;
          context.beginPath();
          context.moveTo(x, y);
          context.lineTo(x, y+h);
          context.lineTo(x+w, y+h);
          context.lineTo(x+w, y);
          context.lineTo(x, y);
          context.fillStyle = colorCssFromTiny(0x08);//TODO foreground colors
          context.fillRect(x, y, w, h);
          context.strokeStyle = goal ? "#ff0" : "#000";
          context.stroke();
          goal = false;
        } break;
        
      case "ONEWAY": {
          if (!gotBgcolor) warn("ONEWAY before BGCOLOR");
          gotOther = true;
          const x = command[1] * VISUAL_SCALE;
          const y = command[2] * VISUAL_SCALE;
          const w = command[3] * VISUAL_SCALE;
          context.fillStyle = colorCssFromTiny(0x92);//TODO foreground colors
          context.fillRect(x, y, w, 8 * VISUAL_SCALE);
          context.beginPath();
          context.moveTo(x, y);
          context.lineTo(x+w, y);
          context.strokeStyle = goal ? "#ff0" : "#000";
          context.stroke();
          goal = false;
        } break;
        
      case "GOAL": {
          goal = true;
        } continue;
        
      case "HERO":
      case "DESMOND":
      case "SUSIE": 
      case "FIRE": 
      case "DUMMY": {
          const dstx = command[1] * VISUAL_SCALE;
          const dsty = command[2] * VISUAL_SCALE;
          let srcx = 0;
          const srcy = 0;
          switch (command[0]) {
            case "HERO": srcx = 0; break;
            case "DESMOND": srcx = 32; break;
            case "SUSIE": srcx = 64; break;
            case "FIRE": srcx = 96; break;
            case "DUMMY": srcx = 128; break;
          }
          context.drawImage(spritesImage, srcx, srcy, 32, 32, dstx - 16, dsty - 32, 32, 32);
        } break;
    }
    if (goal) {
      warn("GOAL was not immediately before a SOLID or ONEWAY");
      goal = false;
    }
  }
}

/* Table of commands.
 *************************************************************/
 
function addCommandButton(td, label, cb) {
  const button = document.createElement("INPUT");
  button.type = "button";
  button.value = label;
  button.addEventListener("click", cb);
  td.appendChild(button);
}
 
function populateUiForCommand(td, command, map) {
  
  const nameSpan = document.createElement("SPAN");
  nameSpan.innerText = command[0];
  td.appendChild(nameSpan);
  
  for (let p=1; p<command.length; p++) {
    const button = document.createElement("INPUT");
    button.type = "button";
    button.value = command[p];
    button.addEventListener("click", () => onEditParameter(map, command, p));
    td.appendChild(button);
  }
}
 
function rebuildTable(map) { // Map
  const table = document.getElementById("commandsTable");
  table.innerHTML = "";
  
  const controlsRow = document.createElement("TR");
  const controlsCol = document.createElement("TD");
  controlsRow.appendChild(controlsCol);
  controlsCol.setAttribute("colspan", "2");
  const addButton = document.createElement("INPUT");
  controlsCol.appendChild(addButton);
  addButton.type = "button";
  addButton.value = "+";
  addButton.addEventListener("click", () => onAddCommand(map));
  table.appendChild(controlsRow);
  
  for (const command of map.commands) {
    const tr = document.createElement("TR");
    
    const tdButtons = document.createElement("TD");
    tr.appendChild(tdButtons);
    addCommandButton(tdButtons, "X", () => onDeleteCommand(map, command));
    addCommandButton(tdButtons, "^", () => onMoveCommand(map, command, -1));
    addCommandButton(tdButtons, "v", () => onMoveCommand(map, command, 1));
    
    const tdCommand = document.createElement("TD");
    tr.appendChild(tdCommand);
    populateUiForCommand(tdCommand, command, map);
    
    table.appendChild(tr);
  }
}

/* Encoded map.
 **********************************************************/
 
function replaceMap(map) { // ArrayBuffer
  document.getElementById("warnings").innerText = "";
  const mapobj = new Map(map);
  const u8s = new Uint8Array(map);
  const hex = document.getElementById("hex");
  hex.value = Array.from(u8s).map((n) => n.toString(16).padStart(2, '0')).reduce((a, v) => `${a} ${v}`, "");
  drawMap(mapobj);
  rebuildTable(mapobj);
}

function undumpHex(src) { // string => ArrayBuffer
  const spaceless = src.replace(/\s/g, "");
  if (spaceless.length & 1) throw new Error(`odd nybble count`);
  const u8s = new Uint8Array(spaceless.length >> 1);
  for (let srcp=0, dstp=0; srcp<spaceless.length; srcp+=2, dstp++) {
    // grrr parseInting both chars together would wrongly succeed if the second is invalid.
    const hi = parseInt(spaceless[srcp], 16);
    const lo = parseInt(spaceless[srcp+1], 16);
    if (isNaN(hi) || isNaN(lo)) throw new Error(`Not a hexadecimal byte: ${spaceless.substr(srcp, 2)}`);
    u8s[dstp] = (hi<<4) | lo;
  }
  return u8s.buffer;
}

function readMap() { // => ArrayBuffer
  try {
    const hex = document.getElementById("hex");
    const bin = undumpHex(hex.value);
    return bin;
  } catch (e) {
    console.error(`Failed to parse map (TODO report nicer)`, e);
  }
}

function readMapName() {
  const input = document.getElementById("loadButton");
  const value = input?.value || '';
  const split = value.split('\\');
  return split[split.length - 1] || '';
}

/* UI events.
 **********************************************************/
 
function onLoad(event) {
  const files = event.target.files;
  if (files?.length) {
    const url = URL.createObjectURL(files[0]);
    window.fetch(url).then((response) => {
      response.arrayBuffer().then((body) => {
        replaceMap(body);
        window.URL.revokeObjectURL(url);
      });
    }).catch((error) => {
      console.error(`Failed to load map.`, error);
      window.URL.revokeObjectURL(url);
    });
  }
}

function onSave() {
  const name = readMapName() || 'untitled';
  const map = readMap();
  const blob = new window.File([map], name, { type: "application/octet-stream" });
  const url = window.URL.createObjectURL(blob);
  window.open(url).addEventListener("load", () => {
    window.URL.revokeObjectURL(url);
  });
}

function onHexChange() {
  const text = document.getElementById("hex").value;
  const bin = readMap();
  replaceMap(bin);
}

function onDeleteCommand(map, command) {
  const p = map.commands.indexOf(command);
  if (p < 0) return;
  map.commands.splice(p, 1);
  replaceMap(map.encode());
}

function onMoveCommand(map, command, d) {
  let p = map.commands.indexOf(command);
  if (p < 0) return;
  let np = p + d;
  if ((np < 0) || (np >= map.commands.length)) return;
  map.commands.splice(p, 1);
  map.commands.splice(np, 0, command);
  replaceMap(map.encode());
}

function onAddCommand(map) {
  let name = window.prompt("Command:");
  if (!name) return;
  name = name.trim().toUpperCase();
  const meta = COMMANDS.find((c) => c[0] === name);
  if (!meta) {
    window.alert(`Command '${name}' not found.`);
    return;
  }
  const command = [meta[0]];
  if (meta[2] < 0) switch (meta[1]) {
    case MAP_CMD_NOOPN: command.push(0); break;
    default: throw new Error(`Command '${name}' declares variable length but onAddCommand() is not familiar`);
  } else {
    for (let i=0; i<meta[2]; i++) {
      command.push(0);
    }
  }
  map.commands.push(command);
  // Well this is kind of silly... replaceMap() is going to rebuild everything.
  // Assuming it works at all, bite your tongue and tolerate the ridiculous inefficiency.
  replaceMap(map.encode());
}

// (p) is the index in (command), ie 1 for the first parameter
function onEditParameter(map, command, p) {
  let v = window.prompt("Value:", command[p]);
  if (!v) return;
  v = +v;
  if (isNaN(v)) return;
  if (v === command[p]) return;
  command[p] = v;
  replaceMap(map.encode());
}

/* Setup.
 *************************************************************/

function buildUi() {
  const parent = document.body;
  parent.innerHTML = "";
  
  const warnings = document.createElement("DIV");
  warnings.id = "warnings";
  parent.appendChild(warnings);
  
  const loadButton = document.createElement("INPUT");
  loadButton.id = "loadButton";
  loadButton.setAttribute("type", "file");
  loadButton.addEventListener("change", (event) => onLoad(event));
  parent.appendChild(loadButton);
  
  const saveButton = document.createElement("INPUT");
  saveButton.setAttribute("type", "button");
  saveButton.setAttribute("value", "Save");
  saveButton.addEventListener("click", onSave);
  parent.appendChild(saveButton);
  
  const hex = document.createElement("TEXTAREA");
  hex.id = "hex";
  hex.addEventListener("change", () => onHexChange());
  parent.appendChild(hex);
  
  const visual = document.createElement("CANVAS");
  visual.id = "visual";
  visual.width = 96 * VISUAL_SCALE;
  visual.height = 64 * VISUAL_SCALE;
  parent.appendChild(visual);
  
  const commandsTable = document.createElement("TABLE");
  commandsTable.id = "commandsTable";
  parent.appendChild(commandsTable);
}

window.addEventListener("load", () => {
  buildUi();
});
