from setuptools import setup, find_packages

dev_requires = [
    "mypy>=1.11.0, <1.12.0",
    "ruff>=0.5.4, <0.6",
]

setup(
    name="lox",
    version="0.1.0",
    author="Vittorio Dinovi",
    author_email="vito.dinovi@gmail.com",
    description="Tooling for clox",
    url="https://github.com/vdinovi/clox",
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    python_requires=">=3.12.3, >3",
    install_requires=[],
    entry_points={
        "console_scripts": [
            "lox = lox.cli:main",
        ],
    },
    extras_require={
        "dev": dev_requires,
    },
)
