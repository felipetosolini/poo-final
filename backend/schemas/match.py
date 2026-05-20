from datetime import datetime
from typing import Optional
from pydantic import BaseModel, ConfigDict


class AnalysisOut(BaseModel):
    model_config = ConfigDict(from_attributes=True)

    id: int
    move_number: int
    eval_cp: Optional[int]
    eval_type: Optional[str]
    best_move: Optional[str]
    classification: Optional[str]


class MatchCreate(BaseModel):
    pgn: str
    result: str
    accuracy_white: Optional[float] = None
    accuracy_black: Optional[float] = None
    analysis: list["AnalysisCreate"] = []


class AnalysisCreate(BaseModel):
    move_number: int
    eval_cp: Optional[int] = None
    eval_type: Optional[str] = None
    best_move: Optional[str] = None
    classification: Optional[str] = None


class MatchOut(BaseModel):
    model_config = ConfigDict(from_attributes=True)

    id: int
    user_id: int
    pgn: str
    result: str
    played_at: datetime
    accuracy_white: Optional[float]
    accuracy_black: Optional[float]
    analysis: list[AnalysisOut] = []


MatchCreate.model_rebuild()
