from datetime import datetime, timezone
from sqlalchemy import String, Integer, Float, Text, DateTime, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column, relationship
from core.database import Base


class Match(Base):
    __tablename__ = "matches"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"), nullable=False, index=True)
    pgn: Mapped[str] = mapped_column(Text, nullable=False)
    result: Mapped[str] = mapped_column(String(10), nullable=False)  # "1-0", "0-1", "1/2-1/2"
    played_at: Mapped[datetime] = mapped_column(
        DateTime, default=lambda: datetime.now(timezone.utc)
    )
    accuracy_white: Mapped[float | None] = mapped_column(Float, nullable=True)
    accuracy_black: Mapped[float | None] = mapped_column(Float, nullable=True)

    user: Mapped["User"] = relationship("User", back_populates="matches")  # noqa: F821
    analysis: Mapped[list["Analysis"]] = relationship("Analysis", back_populates="match")


class Analysis(Base):
    __tablename__ = "analysis"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    match_id: Mapped[int] = mapped_column(ForeignKey("matches.id"), nullable=False, index=True)
    move_number: Mapped[int] = mapped_column(Integer, nullable=False)
    eval_cp: Mapped[int | None] = mapped_column(Integer, nullable=True)
    eval_type: Mapped[str | None] = mapped_column(String(10), nullable=True)  # "cp" o "mate"
    best_move: Mapped[str | None] = mapped_column(String(10), nullable=True)
    classification: Mapped[str | None] = mapped_column(String(20), nullable=True)

    match: Mapped["Match"] = relationship("Match", back_populates="analysis")
