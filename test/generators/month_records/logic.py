from __future__ import annotations

import calendar
import datetime as dt
from typing import Iterable


MONDAY = 0
WEDNESDAY = 2
FRIDAY = 4
TARGET_WEEKDAYS = {MONDAY, WEDNESDAY, FRIDAY}


def validate_year_month(year: int, month: int) -> None:
    if year < 1000 or year > 9999:
        raise ValueError("--year must be a 4-digit value (1000-9999).")
    if month < 1 or month > 12:
        raise ValueError("--month must be between 1 and 12.")


def iter_session_days(year: int, month: int) -> Iterable[dt.date]:
    _, last_day = calendar.monthrange(year, month)
    for day in range(1, last_day + 1):
        current = dt.date(year, month, day)
        if current.weekday() in TARGET_WEEKDAYS:
            yield current


def resolve_session_days(year: int, month: int, active_days: int | None) -> list[dt.date]:
    session_days = list(iter_session_days(year, month))
    if active_days is None:
        return session_days
    if active_days <= 0:
        raise ValueError("--active-days must be a positive integer.")
    if active_days > len(session_days):
        raise ValueError(
            f"--active-days={active_days} exceeds available training days "
            f"({len(session_days)}) for {year:04d}-{month:02d}."
        )
    return session_days[:active_days]


def build_content(
    year: int,
    month: int,
    exercise_keys: list[str],
    active_days: int | None = None,
) -> str:
    lines: list[str] = [f"y{year:04d}", f"m{month:02d}"]

    for session_index, session_day in enumerate(
        resolve_session_days(year, month, active_days)
    ):
        lines.append("")
        lines.append(session_day.strftime("%m%d"))

        for exercise_index, exercise in enumerate(exercise_keys):
            w1 = 40 + 10 * exercise_index + 2 * session_index
            w2 = w1 + 10
            lines.append(exercise)
            lines.append(f"+{w1} 10+8+6")
            lines.append(f"+{w2} 8+6+4")
            lines.append("")

    while lines and lines[-1] == "":
        lines.pop()
    return "\n".join(lines) + "\n"
