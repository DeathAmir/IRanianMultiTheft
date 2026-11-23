use std::process::Command;
use tauri::Manager;

fn game_executable(app: &tauri::AppHandle) -> Result<std::path::PathBuf, String> {
    let resource_dir = app.path().resource_dir().map_err(|e| e.to_string())?;
    let executable = resource_dir.join("game").join("irantheft.exe");
    if !executable.is_file() {
        return Err("Game executable was not found in the installation.".to_string());
    }
    Ok(executable)
}

#[tauri::command]
fn game_ready(app: tauri::AppHandle) -> bool {
    game_executable(&app).is_ok()
}

#[tauri::command]
fn launch_game(app: tauri::AppHandle) -> Result<(), String> {
    let executable = game_executable(&app)?;
    let working_dir = executable.parent().ok_or_else(|| "Invalid game directory.".to_string())?;
    Command::new(executable)
        .current_dir(working_dir)
        .spawn()
        .map_err(|e| format!("Could not start the game: {e}"))?;
    Ok(())
}

#[tauri::command]
fn open_website() -> Result<(), String> {
    open_url("https://irautox.ir")
}

#[tauri::command]
fn open_source() -> Result<(), String> {
    open_url("https://github.com/DeathAmir/IRanianMultiTheft")
}

fn open_url(url: &str) -> Result<(), String> {
    #[cfg(target_os = "windows")]
    {
        Command::new("cmd").args(["/C", "start", "", url]).spawn().map_err(|e| e.to_string())?;
    }
    #[cfg(target_os = "linux")]
    {
        Command::new("xdg-open").arg(url).spawn().map_err(|e| e.to_string())?;
    }
    #[cfg(target_os = "macos")]
    {
        Command::new("open").arg(url).spawn().map_err(|e| e.to_string())?;
    }
    Ok(())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![game_ready, launch_game, open_website, open_source])
        .run(tauri::generate_context!())
        .expect("failed to run IRAntheft launcher");
}
