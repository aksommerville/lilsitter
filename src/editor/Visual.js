/* Visual.js
 * Manages display of a Map in a Canvas element.
 */
 
import { COMMANDS, isSpriteCommandName } from "./Map.js";
import { colorCssFromTiny } from "./color.js";
 
export const SCALE = 4;

const spritesImage = new Image();
spritesImage.src = "/sprites.png";
 
export class Visual {

  /* Setup.
   **********************************************************************/

  constructor() {
    this.canvas = null; // HTMLCanvasElement
    this.map = null; // Map
    this.highlight = null; // One from map.commands
    this.dragging = null; // One from map.commands; while mouse down
    this.dragRecent = [0,0]; // [x,y] game coords for the last mouse position while dragging
    this.dragSize = false; // If true, we are editing w/h, not x/y
    this.mapListener = null;
    this.onhighlight = (command) => {}; // Caller may set directly.
  }
  
  attachToDom(canvas) {
    this.canvas = canvas;
    if (!canvas) return;
    this.canvas.addEventListener("mousemove", (event) => this.onMouseMove(event));
    this.canvas.addEventListener("mouseleave", (event) => this.onMouseMove(event));
    this.canvas.addEventListener("mouseup", (event) => this.onMouseUp(event));
    this.canvas.addEventListener("mousedown", (event) => this.onMouseDown(event));
    this.canvas.addEventListener("contextmenu", (event) => this.onContextMenu(event));
    this.draw();
  }
  
  replaceMap(map) {
    this.map?.unlisten(this.mapListener);
    this.map = map;
    this.mapListener = this.map?.listen(() => this.onMapChanged());
    this.draw();
  }
  
  /* Draw.
   ************************************************************************/
  
  draw() {
    if (!this.canvas) return;
    const context = this.canvas.getContext("2d");
    if (!this.map) {
      context.fillStyle = "#000";
      context.fillRect(0, 0, this.canvas.width, this.canvas.height);
      return;
    }
    let goal = false;
    for (const command of this.map.commands) {
      if (command[0] === 0x06) {
        goal = true;
        continue;
      }
      this.drawCommand(context, command, goal, command === this.highlight);
      goal = false;
    }
  }
  
  drawCommand(context, command, goal, highlight) {
    const opname = COMMANDS[command[0]]?.[0];
    switch (opname) {
 
      case "BGCOLOR": {
          context.fillStyle = colorCssFromTiny(command[1]);
          context.fillRect(0, 0, this.canvas.width, this.canvas.height);
        } break;
 
      case "SOLID": {
          const x = command[1] * SCALE;
          const y = command[2] * SCALE;
          const w = command[3] * SCALE;
          const h = command[4] * SCALE;
          if (highlight) {
            context.fillStyle = "#ff0";
            context.fillRect(x - 1, y - 1, w + 2, h + 2);
          }
          context.fillStyle = goal ? "#840" : "#060";
          context.fillRect(x, y, w, h);
        } break;
 
      case "ONEWAY": {
          const x = command[1] * SCALE;
          const y = command[2] * SCALE;
          const w = command[3] * SCALE;
          if (highlight) {
            context.fillStyle = "#ff0";
            context.fillRect(x - 1, y - 1, w + 2, 3);
          }
          context.setLineDash(goal ? [3, 3] : [1, 1]);
          context.beginPath();
          context.moveTo(x, y);
          context.lineTo(x + w - 1, y);
          context.strokeStyle = goal ? "#800" : "#000";
          context.stroke();
          context.setLineDash([]);
        } break;
        
      default: if (isSpriteCommandName(opname)) {
          const dstx = command[1] * SCALE - 16;
          const dsty = command[2] * SCALE - 32;
          let srcx = 0, srcy = 0;
          switch (opname) {
            case "HERO": srcx = 32; break;
            case "DESMOND": srcx = 64; break;
            case "SUSIE": srcx = 96; break;
            case "FIRE": srcx = 128; break;
            case "DUMMY": srcx = 160; break;
            case "CROCBOT": srcx = 192; break;
            case "PLATFORM": srcx = 224; break;
          }
          if (highlight) {
            context.fillStyle = "#ff0";
            context.fillRect(dstx - 1, dsty - 1, 34, 34);
          }
          context.drawImage(spritesImage, srcx, srcy, 32, 32, dstx, dsty, 32, 32);
        }
    }
  }
  
  coordsGameFromEvent(event) { // => [x,y] in [0,0]..[95,63]
    if (!this.canvas) return [0, 0];
    const bounds = this.canvas.getBoundingClientRect();
    return [
      Math.floor((event.clientX - bounds.x) / SCALE),
      Math.floor((event.clientY - bounds.y) / SCALE)
    ];
  }
  
  /* UI events.
   ****************************************************************/
   
  highlightCommand(command) {
    if (command === this.highlight) return;
    this.highlight = command;
    this.draw();
    this.onhighlight(command);
  }
   
  onMouseMove(event) {
    if (!this.map) return;
    const [x, y] = this.coordsGameFromEvent(event);
    
    if (this.dragging) {
      const dx = x - this.dragRecent[0];
      const dy = y - this.dragRecent[1];
      if (!dx && !dy) return;
      this.dragRecent = [x, y];
      if (this.dragSize) this.map.adjustCommandDimensions(this.dragging, dx, dy);
      else this.map.moveCommandSpatially(this.dragging, dx, dy);
      // No need to draw. If it really changed, we will get an onMapChanged event.
      
    } else {
      const command = this.map.findCommandAtPoint(x, y);
      this.highlightCommand(command);
    }
  }
   
  onMouseDown(event) {
    if (!this.map) return;
    const [x, y] = this.coordsGameFromEvent(event);
    const command = this.map.findCommandAtPoint(x, y);
    if (!command) return;
    
    // Right-click, if the command has "w" or "h" parameters, operate on those instead.
    this.dragSize = false;
    if (event.button === 2) {
      const meta = COMMANDS[command[0]];
      const wix = meta.indexOf("w");
      const hix = meta.indexOf("h");
      if ((wix >= 0) || (hix >= 0)) {
        this.dragSize = true;
      }
    }
    
    this.dragging = command;
    this.highlight = command;
    this.dragRecent = [x, y];
    this.draw();
  }
   
  onMouseUp(event) {
    if (!this.map) return;
    if (!this.dragging) return;
    const [x, y] = this.coordsGameFromEvent(event);
    this.dragging = null;
    this.draw();
  }
   
  onContextMenu(event) {
    event.preventDefault();
  }
  
  onMapChanged() {
    this.draw();
  }

}
