/* Index is opcode.
 * [0]: name
 * [1..]: parameter names
 */
export const COMMANDS = [
  ["EOF"],
  ["NOOP1"],
  ["RSV1"], // NOOPN in the app, but we're not using it, will remove from app
  ["BGCOLOR","color"],
  ["SOLID","x","y","w","h"],
  ["ONEWAY","x","y","w"],
  ["GOAL",],
  ["HERO","x","y"],
  ["DESMOND","x","y"],
  ["SUSIE","x","y"],
  ["FIRE","x","y"],
  ["DUMMY","x","y","tileid"],
  ["CROCBOT","x","y"],
  ["PLATFORM","x","y","mode"],
];

export function isSpriteCommandName(name) {
  switch (name) {
    case "HERO":
    case "DESMOND":
    case "SUSIE":
    case "FIRE":
    case "DUMMY":
    case "CROCBOT":
    case "PLATFORM":
      return true;
  }
  return false;
}

export class Map {

  /* bin: ArrayBuffer | null
   */
  constructor(bin) {
    this.commands = []; // [opcode,...params] numbers 0..255
    this.listeners = []; // {token,cb}
    this.nextListenerToken = 1;
    this.bin = bin || new ArrayBuffer(0);
    this.decode();
  }
  
  /* Clears all live state and rebuilds from (bin).
   */
  decode() {
    this.commands = [];
    const src = new Uint8Array(this.bin);
    for (let p=0; p<src.length; ) {
      const opcode = src[p++];
      const meta = COMMANDS[opcode];
      if (!meta) throw new Error(`Unknown map command ${opcode} at ${p-1}/${src.length}`);
      const command = [opcode];
      if (p + meta.length - 1 > src.length) {
        throw new Error(`Command ${opcode} overruns input, expecting ${meta.length - 1} bytes param`);
      }
      for (let i=meta.length-1; i-->0; ) {
        command.push(src[p++]);
      }
      this.commands.push(command);
    }
  }
  
  /* => ArrayBuffer
   */
  encode() {
    const len = this.commands.reduce((a, v) => a + v.length, 0);
    const dst = new Uint8Array(len);
    let p = 0;
    for (const command of this.commands) {
      for (const v of command) {
        dst[p++] = v;
      }
    }
    if (p !== dst.length) {
      throw new Error(`Miscalculated output length. len=${len} dst.length=${dst.length} p=${p}`);
    }
    return dst.buffer;
  }
  
  /* => [opcode, ...params] or null
   * Returns the frontmost command near this point.
   * Every visual command has a minimum 3x3 size (eg ONEWAY are 3 pixels high), to ensure reachable.
   */
  findCommandAtPoint(x, y) {
    let matched = null;
    const box = (bx, by, bw, bh) => {
      if (bw < 3) { bx -= 2; bw += 4; }
      if (bh < 3) { by -= 2; bh += 4; }
      if (x < bx) return false;
      if (y < by) return false;
      if (x >= bx + bw) return false;
      if (y >= by + bh) return false;
      return true;
    };
    for (const command of this.commands) {
      const opname = COMMANDS[command[0]][0];
      switch (opname) {
        case "SOLID": if (box(command[1], command[2], command[3], command[4])) matched = command; break;
        case "ONEWAY": if (box(command[1], command[2] - 1, command[3], 3)) matched = command; break;
        default: if (isSpriteCommandName(opname)) {
            // Sprites can be any size, but this editor renders them all 32x32, ie 8x8 game pixels.
            if (box(command[1] - 4, command[2] - 8, 8, 8)) matched = command;
          }
      }
    }
    return matched;
  }
  
  /* For commands with a position in space, shift it by (dx,dy) game pixels.
   * Modifies (command) in place.
   */
  moveCommandSpatially(command, dx, dy) {
    if (!command) return;
    const opname = COMMANDS[command[0]]?.[0];
    switch (opname) {
      case "SOLID": {
          command[1] = Math.min(255, Math.max(0, command[1] + dx));
          command[2] = Math.min(255, Math.max(0, command[2] + dy));
          this.broadcast();
        } return true;
      case "ONEWAY": {
          command[1] = Math.min(255, Math.max(0, command[1] + dx));
          command[2] = Math.min(255, Math.max(0, command[2] + dy));
          this.broadcast();
        } return true;
      default: if (isSpriteCommandName(opname)) {
          // All sprites begin with (x,y), and we depend on it.
          command[1] = Math.min(255, Math.max(0, command[1] + dx));
          command[2] = Math.min(255, Math.max(0, command[2] + dy));
          this.broadcast();
          return true;
        }
    }
    return false;
  }
  
  /* Like moveCommandSpatially(), but (w,h) instead of (x,y).
   */
  adjustCommandDimensions(command, dx, dy) {
    if (!command) return;
    const opname = COMMANDS[command[0]]?.[0];
    switch (opname) {
      case "SOLID": {
          command[3] = Math.min(255, Math.max(0, command[3] + dx));
          command[4] = Math.min(255, Math.max(0, command[4] + dy));
          this.broadcast();
        } return true;
      case "ONEWAY": {
          command[1] = Math.min(255, Math.max(0, command[1] + dx));
          this.broadcast();
        } return true;
    }
    return false;
  }
  
  moveCommandInList(command, d) {
    if (!command || !d) return;
    const op = this.commands.indexOf(command);
    if (op < 0) return;
    const np = op + d;
    if (np < 0) return;
    if (np >= this.commands.length) return;
    this.commands.splice(op, 1);
    this.commands.splice(np, 0, command);
    this.broadcast();
  }
  
  /* We call (cb) whenever something changes.
   * Returns a token for unlistening.
   */
  listen(cb) {
    const token = this.nextListenerToken++;
    this.listeners.push({token, cb});
    return token;
  }
  
  unlisten(token) {
    const p = this.listeners.findIndex((l) => l.token === token);
    if (p >= 0) {
      this.listeners.splice(p, 1);
    }
  }
  
  broadcast() {
    for (const listener of this.listeners) {
      listener.cb();
    }
  }
  
  /* Add a command at the end of our list.
   * You supply either the opcode or name, presumably straight from the user.
   * New command will have sensible defaults.
   */
  addCommand(opname) {
  
    let meta = null;
    if (typeof(opname) === "number") {
      // Number must be an exact opcode, which is also the index in COMMANDS.
      meta = COMMANDS[opname];
    } else if (typeof(opname) === "string") {
      opname = opname.trim().toUpperCase();
      if (!(meta = COMMANDS.find((c) => c[0] === opname))) {
        meta = COMMANDS[+opname];
      }
    } else {
      return null;
    }
    if (!meta) return null;
    
    const cmd = meta.map(() => 0);
    cmd[0] = COMMANDS.indexOf(meta);
    
    switch (meta[0]) {
      case "BGCOLOR": cmd[1] = 209; break;
      case "SOLID": cmd[1] = 44; cmd[2] = 28; cmd[3] = 8; cmd[4] = 8; break;
      case "ONEWAY": cmd[1] = 44; cmd[2] = 32; cmd[3] = 8; break;
      case "HERO": cmd[1] = 48; cmd[2] = 36; break;
      case "DESMOND": cmd[1] = 48; cmd[2] = 36; break;
      case "SUSIE": cmd[1] = 48; cmd[2] = 36; break;
      case "FIRE": cmd[1] = 48; cmd[2] = 36; break;
      case "DUMMY": cmd[1] = 48; cmd[2] = 36; break;
      case "CROCBOT": cmd[1] = 48; cmd[2] = 36; break;
      case "PLATFORM": cmd[1] = 48; cmd[2] = 36; break;
    }
    
    this.commands.push(cmd);
    this.broadcast();
    return cmd;
  }
  
  removeCommand(command) {
    const index = this.commands.indexOf(command);
    if (index < 0) return;
    this.commands.splice(index, 1);
    this.broadcast();
  }
  
}
