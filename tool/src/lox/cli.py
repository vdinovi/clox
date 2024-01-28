import argparse

from lox.config import Config, load_config
from lox.test import TestRunner


def main() -> None:
    parser = argparse.ArgumentParser(description="lox tool")
    subparsers = parser.add_subparsers(
        title="subcommands",
        description="valid subcommands",
        help="additional help",
        required=True,
    )

    parser_test = subparsers.add_parser("test")
    parser_test.add_argument(
        "--program",
        type=str,
        help="Path to lox program",
        required=True,
    )
    parser_test.add_argument(
        "--test",
        type=str,
        action="append",
        nargs="+",
        help="Path to a test program",
    )
    parser_test.set_defaults(func=test)

    args = parser.parse_args()
    args.func(args, load_config())


def test(args: argparse.Namespace, config: Config) -> None:
    if args.test:
        tests = [test for tests in args.test for test in tests]
    else:
        tests = [path for path in (config.root_dir / "test" / "lox").rglob("*.lox")]

    runner = TestRunner(program=args.program, tests=tests)
    for result in runner.run():
        result.ansi_color = True
        print(result)


if __name__ == "__main__":
    main()
