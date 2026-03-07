import os
from dataclasses import dataclass

@dataclass(frozen=True)
class Settings:
    # OpenAI-compatible base URL, including optional /v1
    # Examples:
    #   Ollama:   http://127.0.0.1:11434/v1
    #   vLLM:     http://127.0.0.1:8000/v1
    #   OpenAI:   https://api.openai.com/v1
    base_url: str = os.getenv("QTLLM_BASE_URL", "http://127.0.0.1:11434/v1")

    # For Ollama/LMS/vLLM often not required; OpenAI requires it.
    api_key: str = os.getenv("QTLLM_API_KEY", "")

    # Model name is provider-specific
    model: str = os.getenv("QTLLM_MODEL", "qwen2.5-coder:7b")

    # Request knobs
    temperature: float = float(os.getenv("QTLLM_TEMPERATURE", "0.2"))
    timeout_s: int = int(os.getenv("QTLLM_TIMEOUT", "120"))

    # Context window hint (not all servers support it the same way)
    # We'll pass it when supported via "max_tokens" only; num_ctx is non-standard
    max_tokens: int = int(os.getenv("QTLLM_MAX_TOKENS", "1024"))

    # Extra headers (optional) e.g. Azure, gateway auth.
    # Format: "Header1=Value1;Header2=Value2"
    extra_headers: str = os.getenv("QTLLM_HEADERS", "")

def parse_headers(kv: str) -> dict:
    """
    Parse QTLLM_HEADERS like: "X-Org=abc;X-User=tom"
    """
    headers = {}
    kv = (kv or "").strip()
    if not kv:
        return headers
    parts = [p.strip() for p in kv.split(";") if p.strip()]
    for p in parts:
        if "=" not in p:
            continue
        k, v = p.split("=", 1)
        headers[k.strip()] = v.strip()
    return headers