import { Map, COMMANDS } from "./Map.js";
import { colorCssFromTiny } from "./color.js";
import { Visual, SCALE as VISUAL_SCALE } from "./Visual.js";
import { CommandList } from "./CommandList.js";

let visual = new Visual();
let commandList = new CommandList();

visual.onhighlight = (command) => commandList.highlightCommand(command);
commandList.onhighlight = (command) => visual.highlightCommand(command);

/* Encoded map.
 **********************************************************/
 
function replaceMap(serial) { // ArrayBuffer
  const map = new Map(serial);
  visual.replaceMap(map);
  commandList.replaceMap(map);
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
  const map = visual.map.encode();
  const blob = new window.File([map], name, { type: "application/octet-stream" });
  const url = window.URL.createObjectURL(blob);
  window.open(url).addEventListener("load", () => {
    window.URL.revokeObjectURL(url);
  });
}

/* Setup.
 *************************************************************/

function buildUi() {
  const parent = document.body;
  parent.innerHTML = "";
  
  const outerRow = document.createElement("DIV");
  outerRow.classList.add("outerRow");
  parent.appendChild(outerRow);
  
  const leftColumn = document.createElement("DIV");
  outerRow.appendChild(leftColumn);
  
  const rightColumn = document.createElement("DIV");
  outerRow.appendChild(rightColumn);
  
  const filesSection = document.createElement("DIV");
  leftColumn.appendChild(filesSection);
  
  const loadButton = document.createElement("INPUT");
  loadButton.id = "loadButton";
  loadButton.setAttribute("type", "file");
  loadButton.addEventListener("change", (event) => onLoad(event));
  filesSection.appendChild(loadButton);
  
  const saveButton = document.createElement("INPUT");
  saveButton.setAttribute("type", "button");
  saveButton.setAttribute("value", "Save");
  saveButton.addEventListener("click", onSave);
  filesSection.appendChild(saveButton);
  
  const visualElement = document.createElement("CANVAS");
  visualElement.id = "visual";
  visualElement.width = 96 * VISUAL_SCALE;
  visualElement.height = 64 * VISUAL_SCALE;
  visual.attachToDom(visualElement);
  leftColumn.appendChild(visualElement);
  
  const commandsContainer = document.createElement("DIV");
  commandList.attachToDom(commandsContainer);
  rightColumn.appendChild(commandsContainer);
}

window.addEventListener("load", () => {
  buildUi();
  replaceMap(new ArrayBuffer(0));
});
