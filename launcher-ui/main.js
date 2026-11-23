const state = document.querySelector("#game-state");
const play = document.querySelector("#play");
const invoke = window.__TAURI__?.core?.invoke;
async function refresh() {
  if (!invoke) {
    state.textContent = "DESKTOP APP REQUIRED";
    play.disabled = true;
    return;
  }
  try {
    const ready = await invoke("game_ready");
    state.textContent = ready ? "GAME FILES VERIFIED" : "GAME FILES MISSING";
    play.disabled = !ready;
    play.innerHTML = ready ? "<span>▶</span> PLAY NOW" : "<span>!</span> FILES NOT FOUND";
  } catch {
    state.textContent = "GAME CHECK FAILED";
    play.disabled = true;
  }
}
play.addEventListener("click", async () => {
  play.disabled = true;
  state.textContent = "STARTING GAME";
  try {
    await invoke("launch_game");
    state.textContent = "GAME LAUNCHED";
  } catch (error) {
    state.textContent = String(error).slice(0, 90);
  }
  play.disabled = false;
});
document.querySelector("#website").addEventListener("click", () => invoke?.("open_website"));
document.querySelector("#source").addEventListener("click", event => {
  event.preventDefault();
  invoke?.("open_source");
});
refresh();
