from pathlib import Path
import os
import shutil

from ..core.executor import run_command


class AndroidCommand:
    def __init__(self, repo_root: Path):
        self.repo_root = repo_root
        self.android_root = repo_root / "apps" / "workout_android"
        self.shared_mapping = repo_root / "apps" / "config" / "mapping.toml"
        self.android_mapping_asset = (
            self.android_root / "app" / "src" / "main" / "assets" / "mapping.toml"
        )

    def execute(self, action: str, extra_args: list[str] | None = None) -> int:
        wrapper_name = "gradlew.bat" if os.name == "nt" else "gradlew"
        gradle_wrapper = self.android_root / wrapper_name
        if not gradle_wrapper.exists():
            print(f"Error: Gradle wrapper not found: {gradle_wrapper}")
            return 1

        if not self._sync_shared_mapping_to_android_assets():
            return 1

        task = self._resolve_task(action)
        if task is None:
            print(f"Error: unknown android action: {action}")
            return 1

        filtered_args = [arg for arg in (extra_args or []) if arg != "--"]
        command = [str(gradle_wrapper), task] + filtered_args
        return run_command(command, cwd=str(self.android_root))

    def _sync_shared_mapping_to_android_assets(self) -> bool:
        if not self.shared_mapping.exists():
            print(f"Error: shared mapping not found: {self.shared_mapping}")
            return False

        self.android_mapping_asset.parent.mkdir(parents=True, exist_ok=True)

        source_bytes = self.shared_mapping.read_bytes()
        if self.android_mapping_asset.exists():
            target_bytes = self.android_mapping_asset.read_bytes()
            if target_bytes == source_bytes:
                return True

        shutil.copy2(self.shared_mapping, self.android_mapping_asset)
        print(
            "Android mapping synced: "
            f"{self.shared_mapping} -> {self.android_mapping_asset}"
        )
        return True

    @staticmethod
    def _resolve_task(action: str) -> str | None:
        mapping = {
            "assemble-debug": ":app:assembleDebug",
            "assemble-release": ":app:assembleRelease",
            "native-debug": ":app:externalNativeBuildDebug",
            "native-release": ":app:externalNativeBuildRelease",
            "test-debug": ":app:testDebugUnitTest",
        }
        return mapping.get(action)
