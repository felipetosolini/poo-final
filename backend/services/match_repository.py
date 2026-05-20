from typing import Optional
from sqlalchemy.orm import Session
from models.match import Match, Analysis
from schemas.match import MatchCreate


class MatchRepository:
    def __init__(self, db: Session):
        self.db = db

    def get_all_by_user(self, user_id: int) -> list[Match]:
        return (
            self.db.query(Match)
            .filter(Match.user_id == user_id)
            .order_by(Match.played_at.desc())
            .all()
        )

    def get_by_id(self, match_id: int) -> Optional[Match]:
        return self.db.query(Match).filter(Match.id == match_id).first()

    def create(self, user_id: int, data: MatchCreate) -> Match:
        match = Match(
            user_id=user_id,
            pgn=data.pgn,
            result=data.result,
            accuracy_white=data.accuracy_white,
            accuracy_black=data.accuracy_black,
        )
        self.db.add(match)
        self.db.flush()  # obtener match.id sin commitear todavía

        for a in data.analysis:
            self.db.add(Analysis(
                match_id=match.id,
                move_number=a.move_number,
                eval_cp=a.eval_cp,
                eval_type=a.eval_type,
                best_move=a.best_move,
                classification=a.classification,
            ))

        self.db.commit()
        self.db.refresh(match)
        return match
