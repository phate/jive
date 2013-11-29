set -e

~/bin/git-sed-apply.sh --files "*.[ch]" \
	-e "s/typedef enum.*;//g" \
	-e "s/\([a-zA-Z0-9_]\+\)\[const\]/const * \1/g" \
	-e "s/const \([a-zA-Z0-9_]\+\) const/const \1/g" \
	-e "s/\([^])>a-zA-Z0-9_]\)\.\([a-zA-Z0-9_]\+\) = /\1\2 : /g" \
	-e "s/[a-zA-Z0-9_]\+ = memmove/memmove/g" \
	-e "s/INT64_MIN/(-0x7fffffffffffffffLL-1)/g" \
	-e "s/INT64_MAX/0x7fffffffffffffffLL/g" \
	-e "s/UINT32_MIN/0x0/g" \
	-e "s/UINT32_MAX/(0xffffffffUL)/g" \
	-e "s/INT32_MIN/(-0x7fffffffL-1)/g" \
	-e "s/INT32_MAX/0x7fffffffL/g" \
	-e 's/PRId64/"lld"/g' \
	-e 's/PRIx64/"llx"/g' \
	-e 's/PRIu64/"llu"/g' \

#	-e "s/\([^])>a-zA-Z0-9_]\)\[\([a-zA-Z0-9_]\+\)\] = /\1/g" \

for F in $(git ls-files | grep "\.[hc]") ; do \
  python scripts/arrayliterals.py $F ; \
done
