from sqlalchemy import func
from sqlalchemy.orm import Session
from models.match import Match, Analysis


class StatisticsRepository:
    def __init__(self, db: Session):
        self.db = db

    def get_accuracy_averages(self, user_id: int) -> dict:
        row = (
            self.db.query(
                func.avg(Match.accuracy_white).label("avg_white"),
                func.avg(Match.accuracy_black).label("avg_black"),
            )
            .filter(Match.user_id == user_id)
            .first()
        )
        return {
            "avg_accuracy_white": round(row.avg_white or 0, 2),
            "avg_accuracy_black": round(row.avg_black or 0, 2),
        }

    def get_accuracy_evolution(self, user_id: int) -> list[dict]:
        rows = (
            self.db.query(Match.id, Match.played_at, Match.accuracy_white, Match.accuracy_black)
            .filter(Match.user_id == user_id)
            .order_by(Match.played_at.asc())
            .all()
        )
        return [
            {
                "match_id": r.id,
                "played_at": r.played_at.isoformat(),
                "accuracy_white": r.accuracy_white,
                "accuracy_black": r.accuracy_black,
            }
            for r in rows
        ]

    def get_blunder_count(self, user_id: int) -> int:
        return (
            self.db.query(func.count(Analysis.id))
            .join(Match, Match.id == Analysis.match_id)
            .filter(Match.user_id == user_id, Analysis.classification == "Blunder")
            .scalar()
            or 0
        )

    def get_match_count(self, user_id: int) -> int:
        return self.db.query(func.count(Match.id)).filter(Match.user_id == user_id).scalar() or 0
