import pathlib
import tomllib


class Config:
    def __init__(self, root_dir: str | pathlib.Path) -> None:
        self.root_dir = pathlib.Path(root_dir)

    @classmethod
    def from_toml(cls, path: pathlib.Path) -> "Config":
        with open(path, "rb") as f:
            data = tomllib.load(f)
        return Config(root_dir=data["core"]["root_dir"])


_FILENAMES = ("lox.toml", ".lox.toml")
_MAX_DEPTH = 4


def load_config() -> Config:
    def search(dir: pathlib.Path, depth: int = 0) -> pathlib.Path | None:
        if depth > _MAX_DEPTH:
            raise RuntimeError("max recursion depth exceeded")
        for filename in _FILENAMES:
            path = dir / filename
            if not path.exists():
                continue
            elif path.is_dir():
                return search(path, depth + 1)
            elif path.is_file():
                return path
        return None

    search_paths = (pathlib.Path.cwd(),)
    for sp in search_paths:
        if (path := search(sp)) is not None:
            match ext := path.suffix:
                case ".toml":
                    return Config.from_toml(path)
                case _:
                    raise ValueError(f"unsupported config format '{ext}'")

    raise FileNotFoundError(
        f"no config file found among search paths ({",".join(str(p) for p in search_paths)})"
    )
