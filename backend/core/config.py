from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    db_url: str = "mysql+pymysql://chessuser:chesspassword@db:3306/chessdb"
    jwt_secret: str = "change_this"
    jwt_algorithm: str = "HS256"
    jwt_expire_minutes: int = 60 * 24  # 24 horas
    openai_api_key: str = ""

    model_config = {"env_file": ".env", "env_file_encoding": "utf-8"}


settings = Settings()
