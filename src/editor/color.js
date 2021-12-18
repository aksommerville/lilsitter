
// common/ma_ctab.c
// grep '^_' src/editor/editor.js | sed -E 's/_\(([0-9a-f]{2}),([0-9a-f]{2}),([0-9a-f]{2})\)/"#\1\2\3",/g'
// But beware: I'm pretty sure the symbols in ma_ctab.c are not the latest, and these might be off.
export const kColorCssFromTiny = [
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
 
export function colorCssFromTiny(tiny) {
  return kColorCssFromTiny[tiny & 0xff];
}
