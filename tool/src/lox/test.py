import typing
import pathlib
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed


class TestRunner:
    MAX_WORKERS = 2

    def __init__(
        self,
        program: str,
        tests: list[str],
        max_workers: int = MAX_WORKERS,
    ) -> None:
        self.program = program
        self.tests = tests
        self.max_workers = max_workers

    def run(self) -> list["Result"]:
        with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
            futures = [
                executor.submit(execute, Request(program=self.program, test=test))
                for test in self.tests
            ]
        return [future.result() for future in as_completed(futures)]


def execute(request: "Request") -> "Result":
    result = Result(str(request.test_path))
    if not request.program_path.exists():
        result.error = FileNotFoundError(
            f"Program file not found: {request.program_path}"
        )
        return result

    if not request.test_path.exists():
        result.error = FileNotFoundError(f"Test file not found: {request.test_path}")
        return result

    test = Test(path=request.test_path)

    proc = subprocess.Popen(
        [request.program_path, test.path, *test.args],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    try:
        stdout, stderr = proc.communicate(
            input=test.read(),
            timeout=request.timeout_seconds,
        )
    except subprocess.TimeoutExpired as ex:
        proc.kill()
        result.error = ex
        return result

    result = test.verify(
        result=result,
        exit_code=proc.returncode,
        stdout=stdout,
        stderr=stderr,
    )
    return result


class Test:
    EXIT_SUCCESS = 0

    def __init__(
        self,
        path: pathlib.Path,
        args: typing.Iterable[str] | None = None,
    ) -> None:
        self.path = path
        self.name = str(path.name)
        self.args = args if args is not None else []

    def __repr__(self) -> str:
        return f"Test[{self.name}](path={self.path},args={self.args})"

    def read(self) -> bytes:
        with open(self.path, "r") as f:
            return f.read().encode("utf-8")

    def verify(
        self,
        result: "Result",
        exit_code: int,
        stdout: bytes,
        stderr: bytes,
    ) -> "Result":
        print(stdout)
        if exit_code != self.EXIT_SUCCESS:
            result.failure = (
                f"Expected return code {self.EXIT_SUCCESS}, got {exit_code}"
            )
        return result


class Request:
    TIMEOUT_SECONDS = 10

    def __init__(
        self,
        program: str,
        test: str,
        timeout_seconds: int = TIMEOUT_SECONDS,
    ) -> None:
        self.program_path = pathlib.Path(program)
        self.test_path = pathlib.Path(test)
        self.timeout_seconds = timeout_seconds

    def __repr__(self) -> str:
        return (
            f"Request(program_path={self.program_path}, "
            f"test_path={self.test_path}, "
            f"timeout_seconds={self.timeout_seconds})"
        )


class Result:
    PASS = "PASS"
    FAIL = "FAIL"
    ERROR = "ERROR"
    IGNORE = "IGNORE"

    ANSI_GREEN = "\033[92m"
    ANSI_RED = "\033[91m"
    ANSI_YELLOW = "\033[93m"
    ANSI_RESET = "\033[0m"

    def __init__(self, name: str, ansi_color: bool = False) -> None:
        self.name = name
        self.ignore: bool = False
        self.failure: str | None = None
        self.error: "Exception | None" = None
        self.ansi_color = ansi_color

    def __repr__(self) -> str:
        return f"Result(failure={self.failure}, error={repr(self.error)}"

    def __str__(self) -> str:
        if self.ignore:
            return self._ignore_text()
        elif self.error is not None:
            return self._error_text()
        elif self.failure is not None:
            return self._failure_text()
        else:
            return self._pass_text()

    def _error_text(self) -> str:
        color, reset = (self.ANSI_RED, self.ANSI_RESET) if self.ansi_color else ("", "")
        return f"( {color}{self.ERROR:^6s}{reset} ) {self.name} {self.error}"

    def _failure_text(self) -> str:
        color, reset = (self.ANSI_RED, self.ANSI_RESET) if self.ansi_color else ("", "")
        return f"( {color}{self.FAIL:^6s}{reset} ) {self.name} {self.failure}"

    def _ignore_text(self) -> str:
        color, reset = (
            (self.ANSI_YELLOW, self.ANSI_RESET) if self.ansi_color else ("", "")
        )
        return f"( {color}{self.IGNORE:^6s}{reset} ) {self.name}"

    def _pass_text(self) -> str:
        color, reset = (
            (self.ANSI_GREEN, self.ANSI_RESET) if self.ansi_color else ("", "")
        )
        return f"( {color}{self.PASS:^6s}{reset} ) {self.name}"
