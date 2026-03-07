import argparse
import sys
from .config import Settings
from .client import OpenAICompatClient
from .prompts import make_user_prompt, task_to_system

def read_stdin_all() -> str:
    return sys.stdin.read()

def main():
    p = argparse.ArgumentParser(prog="qt-llm", description="Qt Creator -> OpenAI-compatible local LLM bridge")
    p.add_argument("--task", required=True, help="explain|refactor|generate|fixbuild or any custom task string")
    p.add_argument("--extra", default="", help="Extra context/instructions (e.g. errors, requirements)")
    p.add_argument("--model", default=None, help="Override model (otherwise QTLLM_MODEL)")
    p.add_argument("--base-url", default=None, help="Override base URL (otherwise QTLLM_BASE_URL)")
    p.add_argument("--api-key", default=None, help="Override API key (otherwise QTLLM_API_KEY)")
    p.add_argument("--temperature", type=float, default=None, help="Override temperature")
    p.add_argument("--max-tokens", type=int, default=None, help="Override max tokens")

    args = p.parse_args()

    # Read selection from stdin (Qt Creator External Tools can send CurrentSelection)
    selection = read_stdin_all()
    if selection is None:
        selection = ""
    selection = selection.strip("\ufeff")  # handle BOM if any
    selection = selection.strip()

    # If no selection, we still allow task+extra to work (Generate can be prompt-only)
    if not selection:
        # Provide minimal placeholder; better than failing hard
        selection = "(no selection provided)"

    s = Settings()
    # Apply overrides
    if args.base_url:
        s = type(s)(**{**s.__dict__, "base_url": args.base_url})
    if args.api_key is not None:
        s = type(s)(**{**s.__dict__, "api_key": args.api_key})
    if args.temperature is not None:
        s = type(s)(**{**s.__dict__, "temperature": args.temperature})
    if args.max_tokens is not None:
        s = type(s)(**{**s.__dict__, "max_tokens": args.max_tokens})

    client = OpenAICompatClient(s)

    system = task_to_system(args.task)
    user_prompt = make_user_prompt(args.task, selection, args.extra)

    messages = [
        {"role": "system", "content": system},
        {"role": "user", "content": user_prompt},
    ]

    try:
        out = client.chat_completions(
            messages=messages,
            model=args.model,
            temperature=args.temperature,
            max_tokens=args.max_tokens,
        )
        sys.stdout.write(out + ("\n" if out and not out.endswith("\n") else ""))
        return 0
    except Exception as e:
        sys.stdout.write(f"[qt-llm error] {type(e).__name__}: {e}\n")
        return 2

if __name__ == "__main__":
    raise SystemExit(main())