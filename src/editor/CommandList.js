/* CommandList.js
 * Manages the list of parsed commands, interactive.
 */
 
import { COMMANDS } from "./Map.js";
 
export class CommandList {

  /* Setup.
   **************************************************************/
   
  constructor() {
    this.element = null; // HTMLDivElement
    this.map = null; // Map
    this.mapListener = null;
    this.commandIds = []; // [id,command]
    this.nextCommandId = 1;
    this.onhighlight = (command) => {}; // Caller may assign directly.
    this.idDiscriminator = 1;
  }
  
  attachToDom(element) {
    this.element = element;
    this.buildUi();
  }
  
  replaceMap(map) {
    this.map?.unlisten(this.mapListener);
    this.map = map;
    this.commandIds = [];
    this.nextCommandId = 1;
    this.mapListener = this.map?.listen(() => this.onMapChanged());
    this.buildUi();
  }
  
  /* Build UI.
   *************************************************************/
   
  buildUi() {
    if (!this.element) return;
    this.element.innerHTML = "";
    
    const table = document.createElement("TABLE");
    table.classList.add("commands");
    
    const actionsRow = document.createElement("TR");
    table.appendChild(actionsRow);
    const actionsCol = document.createElement("TD");
    actionsCol.colspan = 50;
    actionsRow.appendChild(actionsCol);
    this.addActionButton(actionsCol, "+", () => this.onAddCommand(), this.map);
    
    this.element.appendChild(table);
    
    if (this.map) this.updateTableContent();
  }
  
  addActionButton(parent, label, cb, enable=true) {
    const button = document.createElement("INPUT");
    button.type = "button";
    button.value = label;
    if (!enable) button.disabled = true;
    button.addEventListener("click", cb);
    parent.appendChild(button);
    return button;
  }
  
  /* Repopulate the UI from (this.map).
   * I'll try to do it incrementally...
   */
  updateTableContent() {
    const table = this.element.querySelector("table.commands");
    if (!this.map) {
      for (const tr of table.querySelectorAll("tr.command")) {
        tr.remove();
      }
      return;
    }
    
    // Find rows with no corresponding command, and remove them.
    for (const tr of table.querySelectorAll("tr.command")) {
      const command = this.findCommandForRow(tr);
      if (!command || (this.map.commands.indexOf(command) < 0)) {
        tr.remove();
      }
    }
    
    // Every command in the map must have a row. Create if needed, and populate all of them.
    for (const command of this.map.commands) {
      const tr = this.findRowForCommand(command);
      if (tr) this.populateCommandRow(tr, command);
      else this.addCommandRow(command);
    }
    
    // Now a bit more painful... Confirm that our rows' order matches the map's commands.
    // This really should only change when the user clicks UP or DOWN here.
    // That's not such a common thing... So just detect mismatches, and if mismatched, nuke it and rebuild.
    const trs = Array.from(table.querySelectorAll("tr.command"));
    for (let i=0; i<trs.length; i++) {
      const command = this.findCommandForRow(trs[i]);
      const ci = this.map.commands.indexOf(command);
      if ((ci >= 0) && (ci !== i)) { // ">=0" in case something goes wrong, only proceed if it looks correctable
        for (const tr of trs) tr.remove();
        return this.updateTableContent();
      }
    }
  }
  
  findCommandForRow(tr) {
    const id = +tr.getAttribute("data-command-id");
    return this.commandIds.find((cid) => cid[0] === id)?.[1];
  }
  
  findRowForCommand(command) {
    const cid = this.commandIds.find((cid) => cid[1] === command);
    if (!cid) return null;
    return this.element.querySelector(`tr.command[data-command-id='${cid[0]}']`);
  }
  
  populateCommandRow(tr, command) {
    const commandId = +tr.getAttribute("data-command-id");
    
    let tdActions = tr.querySelector("td.actions");
    if (!tdActions) {
      tdActions = document.createElement("TD");
      tdActions.classList.add("actions");
      this.addActionButton(tdActions, "X", () => this.onDeleteCommand(commandId));
      this.addActionButton(tdActions, "^", () => this.onMoveCommand(commandId, -1));
      this.addActionButton(tdActions, "v", () => this.onMoveCommand(commandId, 1));
      tr.appendChild(tdActions);
    }
    
    let tdOpcode = tr.querySelector("td.opcode");
    if (!tdOpcode) {
      tdOpcode = document.createElement("TD");
      tdOpcode.classList.add("opcode");
      tr.appendChild(tdOpcode);
    }
    tdOpcode.innerText = COMMANDS[command[0]][0];
    
    let tdParams = tr.querySelector("td.params");
    if (!tdParams) {
      tdParams = document.createElement("TD");
      tdParams.classList.add("params");
      tr.appendChild(tdParams);
    }
    
    /* Populate parameters.
     * Since opcodes are immutable, we can assume that tdParams either has the inputs or is brand new.
     * But it will never contain UI for a parameter absent from (command).
     */
    for (let pix=1; pix<command.length; pix++) {
      let input = tdParams.querySelector(`input[data-parameter-index='${pix}']`);
      if (!input) {
        input = document.createElement("INPUT");
        input.type = "number";
        input.min = 0;
        input.max = 255;
        input.setAttribute("data-parameter-index", pix);
        input.classList.add("parameter");
        input.id = `p${this.idDiscriminator++}`;
        input.addEventListener("change", () => this.onParameterChanged(command, pix, +input.value));
        const label = document.createElement("LABEL");
        label.setAttribute("for", input.id);
        label.innerText = COMMANDS[command[0]][pix];
        tdParams.appendChild(label);
        tdParams.appendChild(input);
      }
      input.value = command[pix];
    }
  }
  
  addCommandRow(command) {
  
    let cid = this.commandIds.find((c) => c[1] === command);
    if (!cid) {
      cid = [this.nextCommandId++, command];
      this.commandIds.push(cid);
    }
    const id = cid[0];
    
    const table = this.element.querySelector("table.commands");
    const tr = document.createElement("TR");
    tr.classList.add("command");
    tr.setAttribute("data-command-id", id);
    tr.addEventListener("mouseenter", () => this.onMouseEnterCommand(command));
    tr.addEventListener("mouseleave", () => this.onMouseLeaveCommand(command));
    this.populateCommandRow(tr, command);
    table.appendChild(tr);
  }
  
  highlightCommand(command) {
    for (const element of this.element.querySelectorAll(".highlight")) {
      element.classList.remove("highlight");
    }
    const element = this.findRowForCommand(command);
    if (element) {
      element.classList.add("highlight");
    }
  }
  
  /* UI events.
   *************************************************************/
   
  onMouseEnterCommand(command) {
    this.onhighlight(command);
  }
   
  onMouseLeaveCommand(command) {
    this.onhighlight(null);
  }
   
  onMapChanged() {
    this.updateTableContent();
  }
  
  onAddCommand() {
    if (!this.map) return;
    const message = `Opcode: ${COMMANDS.map((c) => c[0]).join(', ')}`;
    const opname = window.prompt(message);
    if (!opname) return;
    const command = this.map.addCommand(opname);
    if (!command) {
      window.alert(`Invalid opcode '${opname}'`);
      return;
    }
  }
  
  onDeleteCommand(commandId) {
    const command = this.commandIds.find((cid) => cid[0] === commandId)?.[1];
    if (!command) return;
    this.map.removeCommand(command);
  }
  
  onMoveCommand(commandId, d) {
    if (!this.map) return;
    const command = this.commandIds.find((cid) => cid[0] === commandId)?.[1];
    this.map.moveCommandInList(command, d);
  }
  
  onParameterChanged(command, pix, v) {
    if ((v < 0) || (v > 255)) return;
    if (v === command[pix]) return;
    command[pix] = v;
    this.map?.broadcast();
  }
  
}
